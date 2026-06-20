// epoll_server.cpp — epoll-based TCP server implementation
//
// Architecture:
//   Main thread: epoll_wait() loop — accepts connections and reads data
//   Worker threads (ThreadPool): parse JSON and call MessageHandler
//
// Uses level-triggered epoll for simplicity (sufficient for course project scale).

#include "epoll_server.h"
#include "protocol.h"
#include "log_utils.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <cstring>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// ── Constructor / Destructor ───────────────────────────────────────

EpollServer::EpollServer(int port, size_t num_workers)
    : port_(port)
    , pool_(num_workers)
{
    // Create epoll instance
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("EpollServer: epoll_create1 failed: " +
                                 std::string(strerror(errno)));
    }

    // Create wakeup pipe for stop()
    if (pipe2(wakeup_pipe_, O_NONBLOCK | O_CLOEXEC) < 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
        throw std::runtime_error("EpollServer: pipe2 failed: " +
                                 std::string(strerror(errno)));
    }

    // Create heartbeat timerfd (fires every 30 seconds)
    heartbeat_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (heartbeat_fd_ < 0) {
        close(epoll_fd_);
        close(wakeup_pipe_[0]);
        close(wakeup_pipe_[1]);
        epoll_fd_ = -1;
        throw std::runtime_error("EpollServer: timerfd_create failed: " +
                                 std::string(strerror(errno)));
    }

    struct itimerspec ts{};
    ts.it_interval.tv_sec = HEARTBEAT_INTERVAL_SEC;  // repeat every 30s
    ts.it_value.tv_sec    = HEARTBEAT_INTERVAL_SEC;   // first fire after 30s
    if (timerfd_settime(heartbeat_fd_, 0, &ts, nullptr) < 0) {
        close(heartbeat_fd_);
        close(epoll_fd_);
        close(wakeup_pipe_[0]);
        close(wakeup_pipe_[1]);
        epoll_fd_ = -1;
        throw std::runtime_error("EpollServer: timerfd_settime failed: " +
                                 std::string(strerror(errno)));
    }
}

EpollServer::~EpollServer() {
    stop();

    // Release all client sessions — ClientSession destructor closes each fd.
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.clear();
    }

    if (heartbeat_fd_ >= 0) close(heartbeat_fd_);
    if (epoll_fd_ >= 0)  close(epoll_fd_);
    if (wakeup_pipe_[0] >= 0) close(wakeup_pipe_[0]);
    if (wakeup_pipe_[1] >= 0) close(wakeup_pipe_[1]);
    if (listen_fd_ >= 0) close(listen_fd_);
}

// ── Handler Registration ───────────────────────────────────────────

void EpollServer::set_message_handler(MessageHandler handler) {
    on_message_ = std::move(handler);
}

void EpollServer::set_disconnect_handler(DisconnectHandler handler) {
    on_disconnect_ = std::move(handler);
}

// ── Event Loop ─────────────────────────────────────────────────────

void EpollServer::run() {
    // Ignore SIGPIPE (broken pipe when client disconnects)
    signal(SIGPIPE, SIG_IGN);

    // Create listening socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        throw std::runtime_error("EpollServer: socket failed: " +
                                 std::string(strerror(errno)));
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(listen_fd_);
        throw std::runtime_error("EpollServer: bind failed on port " +
                                 std::to_string(port_) + ": " + strerror(errno));
    }

    // Listen
    if (listen(listen_fd_, SOMAXCONN) < 0) {
        close(listen_fd_);
        throw std::runtime_error("EpollServer: listen failed: " +
                                 std::string(strerror(errno)));
    }

    // Set listening fd to non-blocking
    set_nonblocking(listen_fd_);

    // Register listening fd with epoll
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);

    // Register wakeup pipe read end
    ev.events = EPOLLIN;
    ev.data.fd = wakeup_pipe_[0];
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_pipe_[0], &ev);

    // Register heartbeat timerfd
    ev.events = EPOLLIN;
    ev.data.fd = heartbeat_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, heartbeat_fd_, &ev);

    std::cout << now_stamp() << " [Server] Listening on port " << port_ << "\n";

    running_ = true;

    // Main event loop
    constexpr int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    while (running_) {
        int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            std::cerr << now_stamp() << " [Server] epoll_wait error: " << strerror(errno) << "\n";
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == wakeup_pipe_[0]) {
                // Stop signal received
                running_ = false;
                break;
            } else if (fd == heartbeat_fd_) {
                // Heartbeat timer fired — read to disarm, then broadcast PING
                uint64_t expirations;
                (void)::read(heartbeat_fd_, &expirations, sizeof(expirations));
                nlohmann::json ping_msg;
                ping_msg["type"] = "PING";
                broadcast(ping_msg);
            } else if (fd == listen_fd_) {
                handle_new_connection();
            } else {
                handle_client_event(fd);
            }
        }
    }

    std::cout << now_stamp() << " [Server] Event loop exited.\n";
}

void EpollServer::stop() {
    if (!running_) return;
    running_ = false;

    // Wake up epoll_wait by writing to the pipe
    if (wakeup_pipe_[1] >= 0) {
        char c = 'x';
        (void)::write(wakeup_pipe_[1], &c, 1);
    }
}

// ── Connection Management ──────────────────────────────────────────

void EpollServer::handle_new_connection() {
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept4(listen_fd_,
                            reinterpret_cast<struct sockaddr*>(&client_addr),
                            &addr_len,
                            SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << now_stamp() << " [Server] accept4 error: " << strerror(errno) << "\n";
        }
        return;
    }

    // Create session
    uint64_t gen = next_generation_++;
    auto session = std::make_shared<ClientSession>(client_fd);
    session->generation = gen;

    // Store client IP for rate limiting
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
    session->ip_address = ip_str;

    // Register with epoll
    struct epoll_event ev{};
    ev.events  = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        std::cerr << now_stamp() << " [Server] epoll_ctl ADD for fd=" << client_fd
                  << " failed: " << strerror(errno) << "\n";
        close(client_fd);
        return;
    }

    // Store session
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_[client_fd] = std::move(session);
    }

    std::cout << now_stamp() << " [Server] New connection from " << ip_str
              << ":" << ntohs(client_addr.sin_port)
              << " (fd=" << client_fd << ")\n";
}

void EpollServer::handle_client_event(int fd) {
    std::shared_ptr<ClientSession> session;

    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = sessions_.find(fd);
        if (it == sessions_.end()) return;
        session = it->second;  // copy shared_ptr to keep session alive
    }

    // Read (drained inside recv_msgs for LT epoll safety) and extract complete messages.
    // Oversized or true error/EOF returns nullopt -> close.
    auto msgs = Protocol::recv_msgs(*session);

    if (!msgs.has_value()) {
        // Connection closed or error (incl. oversized frame)
        std::cout << now_stamp() << " [Server] fd=" << fd << " recv_msgs returned nullopt -> removing client" << std::endl;
        remove_client(fd);
        return;
    }

    // Dispatch each message to worker thread
    for (auto& msg : *msgs) {
        // Light diagnostic for auth path (type only, no secrets)
        if (!session->is_authenticated()) {
            std::string t = msg.value("type", std::string{});
            if (t == "LOGIN" || t == "REGISTER") {
                std::cout << now_stamp() << " [Server] Auth frame received fd=" << fd << " type=" << t << "\n";
            }
        }
        if (on_message_) {
            // Capture shared_ptr copy + generation. The shared_ptr prevents
            // use-after-free if remove_client() runs before the worker executes.
            // Generation check protects against fd reuse after close + rapid accept.
            auto sess_copy = session;
            uint64_t gen_at_dispatch = session->generation;
            pool_.enqueue([this, fd, gen_at_dispatch, msg, sess_copy]() {
                if (sess_copy->generation != gen_at_dispatch) {
                    // fd was recycled; drop stale task
                    return;
                }

                try {
                    on_message_(*sess_copy, msg);
                } catch (const std::exception& e) {
                    std::cerr << now_stamp() << " [Server] Handler exception for fd=" << fd
                              << ": " << e.what() << "\n";
                }
            });
        }
    }
}

void EpollServer::remove_client(int fd) {
    // Remove from epoll (EBADF is harmless — fd may already be invalid)
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0 && errno != EBADF) {
        std::cerr << now_stamp() << " [Server] epoll_ctl DEL for fd=" << fd
                  << " failed: " << strerror(errno) << "\n";
    }

    std::shared_ptr<ClientSession> removed_session;

    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = sessions_.find(fd);
        if (it != sessions_.end()) {
            removed_session = it->second;
            sessions_.erase(it);
            // CRITICAL: Update the session's generation AND bump the global counter.
            // In-flight worker lambdas captured gen_at_dispatch = session->generation
            // and check sess_copy->generation != gen_at_dispatch. If we only bump
            // next_generation_ without updating the session's field, the check
            // compares the same value to itself (always equal) — a no-op.
            removed_session->generation = ++next_generation_;
        }
    }

    // Call disconnect handler before closing fd
    if (removed_session && on_disconnect_) {
        try {
            on_disconnect_(*removed_session);
        } catch (const std::exception& e) {
            std::cerr << now_stamp() << " [Server] Disconnect handler exception: " << e.what() << "\n";
        }
    }

    if (removed_session) {
        std::cout << now_stamp() << " [Server] Client disconnected: "
                  << (removed_session->is_authenticated() ? removed_session->get_username() : "(unauth)")
                  << " (fd=" << fd << ", gen=" << removed_session->generation << ")\n";

        // shutdown() makes in-flight send() calls fail immediately with EPIPE,
        // preventing them from writing to a reused fd. The actual close(fd)
        // happens in ~ClientSession() when the last shared_ptr is released.
        ::shutdown(fd, SHUT_WR);
    }
    // Do NOT close(fd) here — ClientSession destructor handles it.
    // This keeps the fd alive for in-flight send_to_fd()/broadcast() calls
    // that hold a shared_ptr<ClientSession>.
}

void EpollServer::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

// ── Messaging API ──────────────────────────────────────────────────

void EpollServer::broadcast(const nlohmann::json& msg, const std::string& exclude_username) {
    // Copy shared_ptrs under lock, then send WITHOUT holding the lock.
    // The shared_ptr keeps the session (and its fd) alive during send,
    // preventing fd-reuse races where remove_client() closes the fd and
    // a new connection reuses it before send_msg() completes.
    std::vector<std::shared_ptr<ClientSession>> targets;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (const auto& [fd, session] : sessions_) {
            if (!session->is_authenticated()) continue;
            if (!exclude_username.empty() && session->get_username() == exclude_username) continue;
            targets.push_back(session);
        }
    }
    for (const auto& session : targets) {
        std::lock_guard<std::mutex> send_lock(session->send_mutex_);
        Protocol::send_msg(session->fd, msg);
    }
}

bool EpollServer::send_to_fd(int fd, const nlohmann::json& msg) {
    std::shared_ptr<ClientSession> session;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = sessions_.find(fd);
        if (it == sessions_.end()) return false;
        session = it->second;  // copy shared_ptr to keep session alive
    }
    // Send WITHOUT holding sessions_mutex_ — send_msg does blocking I/O and
    // holding sessions_mutex_ here would deadlock with handle_client_event.
    // Lock send_mutex_ to prevent frame interleaving from concurrent sends.
    // The shared_ptr keeps the fd alive; if remove_client() runs concurrently,
    // shutdown(fd, SHUT_WR) makes send() return EPIPE.
    std::lock_guard<std::mutex> send_lock(session->send_mutex_);
    return Protocol::send_msg(session->fd, msg);
}

std::shared_ptr<ClientSession> EpollServer::find_session(const std::string& username) const {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (const auto& [fd, session] : sessions_) {
        if (session->get_username() == username) {
            return session;
        }
    }
    return nullptr;
}

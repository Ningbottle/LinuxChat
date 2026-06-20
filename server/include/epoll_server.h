#pragma once
// epoll_server.h — epoll-based TCP server with ThreadPool dispatch
//
// Architecture:
//   Main thread: epoll_wait() loop — accepts connections and reads data
//   Worker threads (ThreadPool): parse JSON and call MessageHandler

#include "thread_pool.h"
#include "client_session.h"
#include <atomic>
#include <unordered_map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

class EpollServer {
public:
    /// Callback type: called on a worker thread for each received message.
    /// Args: (session reference, parsed JSON message)
    using MessageHandler = std::function<void(ClientSession&, const nlohmann::json&)>;

    /// Callback type: called when a client disconnects.
    using DisconnectHandler = std::function<void(ClientSession&)>;

    /// Constructs the server. Does not start listening yet.
    /// @param port         TCP port to listen on
    /// @param num_workers  Number of worker threads in the pool
    explicit EpollServer(int port, size_t num_workers = 4);

    ~EpollServer();

    // Non-copyable, non-movable
    EpollServer(const EpollServer&) = delete;
    EpollServer& operator=(const EpollServer&) = delete;

    /// Register the message handler (must be set before run()).
    void set_message_handler(MessageHandler handler);

    /// Register the disconnect handler.
    void set_disconnect_handler(DisconnectHandler handler);

    /// Start the event loop. Blocks until stop() is called.
    void run();

    /// Signal the event loop to exit gracefully.
    void stop();

    /// Broadcast a message to all currently authenticated clients.
    void broadcast(const nlohmann::json& msg, const std::string& exclude_username = "");

    /// Send a message to a specific fd.
    /// @return false if fd not found or send failed.
    bool send_to_fd(int fd, const nlohmann::json& msg);

    /// Find the session of an authenticated user by username.
    /// @return shared_ptr to session, or nullptr if not found/not online.
    /// NOTE: This is safer than find_fd() which returned a raw fd that could
    /// become stale between the check and the use. The shared_ptr keeps the
    /// session alive during the caller's operation.
    std::shared_ptr<ClientSession> find_session(const std::string& username) const;

private:
    void handle_new_connection();
    void handle_client_event(int fd);
    void remove_client(int fd);
    void set_nonblocking(int fd);

    static constexpr int HEARTBEAT_INTERVAL_SEC = 30;  ///< PING broadcast interval

    int         port_;
    int         listen_fd_ = -1;
    int         epoll_fd_  = -1;
    int         heartbeat_fd_ = -1;  ///< timerfd for heartbeat PING
    int         wakeup_pipe_[2] = {-1, -1}; ///< pipe for stop() wakeup
    std::atomic<bool> running_{false};  ///< Signal-safe flag for event loop

    uint64_t    next_generation_ = 1;  ///< for ClientSession::generation (fd reuse protection)

    ThreadPool  pool_;

    /// fd -> ClientSession (shared_ptr so worker lambdas can safely
    /// hold a reference even if remove_client() runs concurrently)
    mutable std::mutex sessions_mutex_;
    std::unordered_map<int, std::shared_ptr<ClientSession>> sessions_;

    MessageHandler    on_message_;
    DisconnectHandler on_disconnect_;
};

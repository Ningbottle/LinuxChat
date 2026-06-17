#pragma once
// epoll_server.h — epoll-based TCP server with ThreadPool dispatch
//
// Architecture:
//   Main thread: epoll_wait() loop — accepts connections and reads data
//   Worker threads (ThreadPool): parse JSON and call MessageHandler

#include "thread_pool.h"
#include "client_session.h"
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

    /// Find the fd of an authenticated user by username.
    /// @return -1 if not found or not online.
    int find_fd(const std::string& username) const;

private:
    void handle_new_connection();
    void handle_client_event(int fd);
    void remove_client(int fd);
    void set_nonblocking(int fd);

    int         port_;
    int         listen_fd_ = -1;
    int         epoll_fd_  = -1;
    int         wakeup_pipe_[2] = {-1, -1}; ///< pipe for stop() wakeup
    bool        running_ = false;

    ThreadPool  pool_;

    /// fd → ClientSession
    mutable std::mutex sessions_mutex_;
    std::unordered_map<int, std::unique_ptr<ClientSession>> sessions_;

    MessageHandler    on_message_;
    DisconnectHandler on_disconnect_;
};

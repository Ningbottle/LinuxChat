#pragma once
// client_session.h — Per-connection state for each TCP client

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <unistd.h>  // close()

/// Holds all state associated with a single connected client.
/// One ClientSession is created per accepted fd and destroyed on disconnect.
///
/// IMPORTANT: The fd is closed in the destructor, NOT in remove_client().
/// This ensures in-flight send_to_fd()/broadcast() calls that hold a
/// shared_ptr<ClientSession> can safely complete before the fd is released.
/// shutdown(fd, SHUT_WR) is called eagerly in remove_client() to make
/// in-flight send() calls fail immediately with EPIPE.
///
/// THREAD SAFETY: username is accessed from both the main epoll thread
/// (broadcast filter, diagnostic logging) and worker threads (login/logout
/// handlers). Use get_username()/set_username()/clear_username() to access
/// it safely. The username_mutex_ is NOT held during I/O operations.
struct ClientSession {
    int         fd;           ///< Socket file descriptor
    std::vector<uint8_t> recv_buf; ///< Accumulates raw bytes for frame extraction
    std::atomic<uint64_t> generation{0}; ///< Incremented per new connection on this fd number (detect reuse)
    uint32_t    consecutive_parse_failures = 0; ///< JSON parse error counter
    mutable std::mutex send_mutex_;  ///< Serializes send() calls on this fd (prevents frame interleaving)
    std::string ip_address;  ///< Client IP address (for rate limiting)

    explicit ClientSession(int fd) : fd(fd) {}

    ~ClientSession() {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    // Non-copyable (owns a file descriptor)
    ClientSession(const ClientSession&) = delete;
    ClientSession& operator=(const ClientSession&) = delete;

    /// Thread-safe check for authentication status.
    bool is_authenticated() const {
        std::lock_guard<std::mutex> lock(username_mutex_);
        return !username_.empty();
    }

    /// Thread-safe username getter.
    std::string get_username() const {
        std::lock_guard<std::mutex> lock(username_mutex_);
        return username_;
    }

    /// Thread-safe username setter (used after login).
    void set_username(const std::string& name) {
        std::lock_guard<std::mutex> lock(username_mutex_);
        username_ = name;
    }

    /// Thread-safe username clear (used on logout).
    void clear_username() {
        std::lock_guard<std::mutex> lock(username_mutex_);
        username_.clear();
    }

    /// Thread-safe check for pending login (username being authenticated).
    std::string get_pending_login() const {
        std::lock_guard<std::mutex> lock(pending_login_mutex_);
        return pending_login_;
    }

    /// Thread-safe pending login setter (used during login process).
    void set_pending_login(const std::string& name) {
        std::lock_guard<std::mutex> lock(pending_login_mutex_);
        pending_login_ = name;
    }

    /// Thread-safe pending login clear (used after login completes or fails).
    void clear_pending_login() {
        std::lock_guard<std::mutex> lock(pending_login_mutex_);
        pending_login_.clear();
    }

private:
    std::string username_;              ///< Empty = not yet authenticated
    mutable std::mutex username_mutex_;  ///< Protects username_ access
    std::string pending_login_;          ///< Username being authenticated (for cleanup on disconnect)
    mutable std::mutex pending_login_mutex_;  ///< Protects pending_login_ access
};

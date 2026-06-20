#pragma once
// message_router.h — Extracted message routing logic from main.cpp
//
// Responsibility:
//   - Route incoming messages by "type" field
//   - Handle register, login, logout, broadcast, private, history
//   - Track online users
//
// Design:
//   - Takes Database& for persistence (testable with :memory: DB)
//   - Takes EpollServer* for network operations (can be null for pure logic tests)
//   - Does NOT depend on epoll internals

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>

class Database;
class EpollServer;
struct ClientSession;

class MessageRouter {
public:
    /// Construct with a database reference. Server pointer is set separately
    /// (required because EpollServer is constructed after MessageRouter).
    explicit MessageRouter(Database& db);

    /// Set the server pointer (must be called before any messages are routed).
    void set_server(EpollServer* server);

    // ── Main dispatcher ──────────────────────────────────────────────

    /// Route a message to the appropriate handler based on "type" field.
    /// Authenticated-only types are rejected if session is not authenticated.
    void route(ClientSession& session, const nlohmann::json& msg);

    // ── Individual handlers (public for testing) ─────────────────────

    void handle_register(ClientSession& session, const nlohmann::json& msg);
    void handle_login(ClientSession& session, const nlohmann::json& msg);
    void handle_logout(ClientSession& session);
    void handle_broadcast(ClientSession& session, const nlohmann::json& msg);
    void handle_private(ClientSession& session, const nlohmann::json& msg);
    void handle_history_req(ClientSession& session, const nlohmann::json& msg);

    // ── Online user management ───────────────────────────────────────

    /// Collect all online usernames into a JSON array message.
    nlohmann::json make_user_list_msg();

    /// Broadcast the current online user list to all clients.
    void broadcast_user_list();

    /// Check if a user is currently tracked as online.
    bool is_user_online(const std::string& username) const;

    /// Clean up login reservation for a user (called on disconnect).
    /// If a client disconnects mid-login, this prevents the username from
    /// being permanently stuck in login_in_progress_.
    void cleanup_login_reservation(const std::string& username);

    /// Check if an IP is rate-limited for login attempts.
    /// @return true if the IP is currently blocked.
    bool is_rate_limited(const std::string& ip_address);

    /// Record a failed login attempt for an IP address.
    void record_failed_login(const std::string& ip_address);

    /// Clear login attempts for an IP on successful login.
    void clear_login_attempts(const std::string& ip_address);

private:
    Database&     db_;
    EpollServer*  server_ = nullptr;

    // Online user tracking (moved from globals in main.cpp)
    mutable std::mutex             online_mutex_;
    std::unordered_set<std::string> online_users_;

    // Login reservation: prevents TOCTOU race where two concurrent LOGIN
    // requests for the same username both pass the find_session() check.
    // Locked during the entire login sequence (check + verify + finish_login).
    mutable std::mutex             login_mutex_;
    std::unordered_set<std::string> login_in_progress_;

    // Rate limiting: track failed login attempts per IP to prevent brute force.
    struct LoginAttempt {
        int count = 0;
        std::chrono::steady_clock::time_point first_attempt;
        std::chrono::steady_clock::time_point blocked_until;
    };
    mutable std::mutex rate_limit_mutex_;
    std::unordered_map<std::string, LoginAttempt> login_attempts_;  // IP -> attempts
    static constexpr int MAX_LOGIN_ATTEMPTS = 5;        // Max attempts before block
    static constexpr int BLOCK_DURATION_SEC = 300;      // 5 minutes block

    // ── Helpers ──────────────────────────────────────────────────────

    /// SHA-256 hash -> lowercase hex string (OpenSSL EVP API)
    static std::string sha256_hex(const std::string& input);

    /// Generate a random 16-byte salt as hex string (32 chars)
    static std::string generate_salt();

    /// Hash password with salt: returns "salt_hex:hash_hex"
    static std::string hash_password(const std::string& password);

    /// Verify password against stored "salt_hex:hash_hex" (supports legacy plain hash)
    static bool verify_password(const std::string& password, const std::string& stored);

    /// Current Unix timestamp in seconds
    static int64_t now_timestamp();

    /// Push recent history to a newly logged-in user
    void push_history(int fd, const std::string& username);

    /// Post-login sequence: add to online, send LOGIN_OK, push history,
    /// broadcast user list, notify others.
    void finish_login(ClientSession& session, const std::string& username);
};

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
#include <mutex>
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

private:
    Database&     db_;
    EpollServer*  server_ = nullptr;

    // Online user tracking (moved from globals in main.cpp)
    mutable std::mutex             online_mutex_;
    std::unordered_set<std::string> online_users_;

    // ── Helpers ──────────────────────────────────────────────────────

    /// SHA-256 hash -> lowercase hex string (OpenSSL EVP API)
    static std::string sha256_hex(const std::string& input);

    /// Current Unix timestamp in seconds
    static int64_t now_timestamp();

    /// Push recent history to a newly logged-in user
    void push_history(int fd, const std::string& username);

    /// Post-login sequence: add to online, send LOGIN_OK, push history,
    /// broadcast user list, notify others.
    void finish_login(ClientSession& session, const std::string& username);
};

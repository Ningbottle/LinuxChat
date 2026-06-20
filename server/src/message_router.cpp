// message_router.cpp — Extracted message routing logic from main.cpp
//
// All handle_* functions previously living as statics in main.cpp are now
// member functions of MessageRouter. The global variables g_db, g_server,
// g_online_users, g_online_mutex have been replaced by class members.

#include "message_router.h"
#include "epoll_server.h"
#include "database.h"
#include "protocol.h"
#include "client_session.h"
#include "log_utils.h"

#include <nlohmann/json.hpp>
#include <openssl/evp.h>  // EVP API for SHA-256 (OpenSSL 1.1/3.x)
#include <openssl/rand.h> // RAND_bytes for cryptographic salt generation
#include <openssl/crypto.h> // CRYPTO_memcmp for timing-safe comparison

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

using json = nlohmann::json;

// ── Input Validation Constants ──────────────────────────────────────
// Prevent DoS via oversized inputs and enforce reasonable limits.
static constexpr size_t MAX_USERNAME_LEN = 32;
static constexpr size_t MAX_PASSWORD_LEN = 128;
static constexpr size_t MAX_MESSAGE_LEN  = 4096;

// ── Constructor ─────────────────────────────────────────────────────

MessageRouter::MessageRouter(Database& db)
    : db_(db)
{}

void MessageRouter::set_server(EpollServer* server) {
    server_ = server;
}

// ── Helpers ─────────────────────────────────────────────────────────

std::string MessageRouter::sha256_hex(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return "";
    }
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, input.data(), input.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

// Generate a random 16-byte salt as hex string (32 chars)
// FATAL: RAND_bytes failure is a fatal error because cryptographic randomness
// is essential for password hashing security. A predictable salt would make
// passwords vulnerable to rainbow table attacks.
std::string MessageRouter::generate_salt() {
    unsigned char salt_bytes[16];
    // Use OpenSSL RAND for cryptographic randomness
    if (RAND_bytes(salt_bytes, sizeof(salt_bytes)) != 1) {
        // This is a fatal error — cryptographic randomness is required
        throw std::runtime_error("FATAL: RAND_bytes failed — cannot generate cryptographic salt");
    }
    std::ostringstream oss;
    for (unsigned char b : salt_bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

// Hash password with salt: returns "salt_hex:hash_hex"
std::string MessageRouter::hash_password(const std::string& password) {
    std::string salt = generate_salt();
    std::string salted = salt + ":" + password;
    std::string hash = sha256_hex(salted);
    return salt + ":" + hash;
}

// Verify password against stored "salt_hex:hash_hex"
// Uses CRYPTO_memcmp for timing-safe comparison to prevent timing attacks.
bool MessageRouter::verify_password(const std::string& password, const std::string& stored) {
    // Parse "salt:hash" format
    auto colon_pos = stored.find(':');
    if (colon_pos == std::string::npos || colon_pos + 1 >= stored.size()) {
        // Legacy format (plain SHA-256 hash, no salt) — for backward compatibility
        std::string computed = sha256_hex(password);
        if (computed.size() != stored.size()) return false;
        return CRYPTO_memcmp(computed.data(), stored.data(), computed.size()) == 0;
    }
    std::string salt = stored.substr(0, colon_pos);
    std::string expected_hash = stored.substr(colon_pos + 1);
    std::string salted = salt + ":" + password;
    std::string computed = sha256_hex(salted);
    if (computed.size() != expected_hash.size()) return false;
    return CRYPTO_memcmp(computed.data(), expected_hash.data(), computed.size()) == 0;
}

int64_t MessageRouter::now_timestamp() {
    return static_cast<int64_t>(std::time(nullptr));
}

void MessageRouter::push_history(int fd, const std::string& /*username*/) {
    auto room_history = db_.get_history("__room__", 20);
    json hist_msg = {
        {"type", "HISTORY_RESP"},
        {"to",   "__room__"},
        {"data", room_history}
    };
    // Use safe send (send_to_fd) to avoid use-after-close if client
    // disconnects between LOGIN_OK and this call.
    if (server_) {
        server_->send_to_fd(fd, hist_msg);
    }
}

json MessageRouter::make_user_list_msg() {
    json users_array = json::array();
    {
        std::lock_guard<std::mutex> lock(online_mutex_);
        for (const auto& name : online_users_) {
            users_array.push_back(name);
        }
    }
    return {
        {"type", "USER_LIST"},
        {"data", users_array},
        {"timestamp", now_timestamp()}
    };
}

void MessageRouter::broadcast_user_list() {
    if (!server_) return;
    json msg = make_user_list_msg();
    server_->broadcast(msg);
}

bool MessageRouter::is_user_online(const std::string& username) const {
    std::lock_guard<std::mutex> lock(online_mutex_);
    return online_users_.count(username) > 0;
}

void MessageRouter::cleanup_login_reservation(const std::string& username) {
    if (username.empty()) return;
    std::lock_guard<std::mutex> lock(login_mutex_);
    login_in_progress_.erase(username);
}

bool MessageRouter::is_rate_limited(const std::string& ip_address) const {
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    auto it = login_attempts_.find(ip_address);
    if (it == login_attempts_.end()) return false;

    auto now = std::chrono::steady_clock::now();

    // Check if block period has expired
    if (now > it->second.blocked_until) {
        // Block expired, reset attempts
        login_attempts_.erase(it);
        return false;
    }

    return it->second.count >= MAX_LOGIN_ATTEMPTS;
}

void MessageRouter::record_failed_login(const std::string& ip_address) {
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    auto& attempt = login_attempts_[ip_address];
    auto now = std::chrono::steady_clock::now();

    // Reset if more than 1 minute since first attempt
    if (attempt.count > 0 && now - attempt.first_attempt > std::chrono::minutes(1)) {
        attempt.count = 0;
    }

    if (attempt.count == 0) {
        attempt.first_attempt = now;
    }

    attempt.count++;

    // Block if too many attempts
    if (attempt.count >= MAX_LOGIN_ATTEMPTS) {
        attempt.blocked_until = now + std::chrono::seconds(BLOCK_DURATION_SEC);
    }
}

void MessageRouter::clear_login_attempts(const std::string& ip_address) {
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    login_attempts_.erase(ip_address);
}

void MessageRouter::finish_login(ClientSession& session, const std::string& username) {
    // Set session username (marks as authenticated)
    session.set_username(username);

    // Add to online users
    {
        std::lock_guard<std::mutex> lock(online_mutex_);
        online_users_.insert(username);
    }

    Protocol::send_ok(session.fd, "LOGIN_OK", username);
    std::cout << now_stamp() << " [Handler] User logged in: " << username << "\n";

    // Push history + broadcast user list
    push_history(session.fd, username);
    broadcast_user_list();

    // Notify others
    json notify = {
        {"type", "NOTIFY"},
        {"content", username + " 加入了聊天室"},
        {"timestamp", now_timestamp()}
    };
    if (server_) {
        server_->broadcast(notify, username);
    }
}

// ── Main Dispatcher ─────────────────────────────────────────────────

void MessageRouter::route(ClientSession& session, const json& msg) {
    std::string type = msg.value("type", "");

    // Unauthenticated: only REGISTER, LOGIN, and PONG allowed
    if (!session.is_authenticated()) {
        if (type == "REGISTER") {
            handle_register(session, msg);
        } else if (type == "LOGIN") {
            handle_login(session, msg);
        } else if (type == "PONG") {
            // Silently ignore PONG from unauthenticated clients
            return;
        } else {
            Protocol::send_error(session.fd, "NOT_AUTHENTICATED",
                                 "请先登录或注册");
        }
        return;
    }

    // Authenticated message routing
    if (type == "PONG") {
        // Silently ignore heartbeat PONG response
        return;
    } else if (type == "BROADCAST") {
        handle_broadcast(session, msg);
    } else if (type == "PRIVATE") {
        handle_private(session, msg);
    } else if (type == "HISTORY_REQ") {
        handle_history_req(session, msg);
    } else if (type == "LOGOUT") {
        handle_logout(session);
    } else {
        Protocol::send_error(session.fd, "INVALID_MSG",
                             "未知消息类型: " + type);
    }
}

// ── Individual Handlers ─────────────────────────────────────────────

void MessageRouter::handle_register(ClientSession& session, const json& msg) {
    std::string username = msg.value("from", "");
    std::string password = msg.value("content", "");

    if (username.empty() || password.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少用户名或密码");
        return;
    }

    // Input length validation
    if (username.size() > MAX_USERNAME_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "用户名过长（最多32字符）");
        return;
    }
    if (password.size() > MAX_PASSWORD_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "密码过长（最多128字符）");
        return;
    }

    // Reserve username during registration to prevent concurrent duplicate registrations
    {
        std::lock_guard<std::mutex> lock(login_mutex_);
        if (login_in_progress_.count(username)) {
            Protocol::send_error(session.fd, "ALREADY_LOGGED_IN",
                                 "该账号正在注册中");
            return;
        }
        login_in_progress_.insert(username);
    }

    // Track pending login on session for cleanup on disconnect
    session.set_pending_login(username);

    std::string hash = hash_password(password);
    std::cout << now_stamp() << " [Auth] register hash for " << username
              << " len=" << hash.size() << std::endl;

    if (!db_.register_user(username, hash)) {
        {
            std::lock_guard<std::mutex> lock(login_mutex_);
            login_in_progress_.erase(username);
        }
        session.clear_pending_login();
        Protocol::send_error(session.fd, "USER_EXISTS", "用户名已存在");
        return;
    }

    // Registration successful: auto-login
    std::cout << now_stamp() << " [Handler] User registered: " << username << "\n";
    finish_login(session, username);
    session.clear_pending_login();
    {
        std::lock_guard<std::mutex> lock(login_mutex_);
        login_in_progress_.erase(username);
    }
}

void MessageRouter::handle_login(ClientSession& session, const json& msg) {
    std::string username = msg.value("from", "");
    std::string password = msg.value("content", "");

    if (username.empty() || password.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少用户名或密码");
        return;
    }

    // Input length validation
    if (username.size() > MAX_USERNAME_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "用户名过长（最多32字符）");
        return;
    }
    if (password.size() > MAX_PASSWORD_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "密码过长（最多128字符）");
        return;
    }

    // Rate limiting: check if IP is blocked
    if (!session.ip_address.empty() && is_rate_limited(session.ip_address)) {
        Protocol::send_error(session.fd, "RATE_LIMITED", "登录尝试过多，请稍后再试");
        return;
    }

    // TOCTOU fix: reserve the username atomically. If two concurrent LOGIN
    // requests for the same username arrive, only one enters the login flow.
    // The other gets ALREADY_LOGGED_IN after the first completes.
    {
        std::lock_guard<std::mutex> lock(login_mutex_);
        if (login_in_progress_.count(username)) {
            Protocol::send_error(session.fd, "ALREADY_LOGGED_IN",
                                 "该账号正在登录中");
            return;
        }
        // Also check if already online
        if (server_ && server_->find_session(username) != nullptr) {
            Protocol::send_error(session.fd, "ALREADY_LOGGED_IN",
                                 "该账号已在其他地方登录");
            return;
        }
        login_in_progress_.insert(username);
    }

    // Track pending login on session for cleanup on disconnect
    session.set_pending_login(username);

    // Verify password (outside login_mutex_ to avoid holding it during DB I/O)
    // Use salted verification: get stored hash, then verify with salt
    std::string stored_hash = db_.get_stored_hash(username);
    std::cout << now_stamp() << " [Auth] verify password for " << username
              << " stored_hash_len=" << stored_hash.size() << std::endl;

    if (stored_hash.empty() || !verify_password(password, stored_hash)) {
        // Record failed login attempt for rate limiting
        if (!session.ip_address.empty()) {
            record_failed_login(session.ip_address);
        }
        // Release reservation on failure
        {
            std::lock_guard<std::mutex> lock(login_mutex_);
            login_in_progress_.erase(username);
        }
        session.clear_pending_login();
        Protocol::send_error(session.fd, "WRONG_PASSWORD", "密码错误");
        return;
    }

    // Login successful — finish_login will add to online_users_.
    // Clear rate limiting on success.
    if (!session.ip_address.empty()) {
        clear_login_attempts(session.ip_address);
    }
    // Release reservation after finish_login completes.
    finish_login(session, username);
    session.clear_pending_login();
    {
        std::lock_guard<std::mutex> lock(login_mutex_);
        login_in_progress_.erase(username);
    }
}

void MessageRouter::handle_logout(ClientSession& session) {
    if (!session.is_authenticated()) return;

    std::string username = session.get_username();

    // Remove from online users
    {
        std::lock_guard<std::mutex> lock(online_mutex_);
        online_users_.erase(username);
    }

    std::cout << now_stamp() << " [Handler] User logged out: " << username << "\n";

    // Clear username before broadcast (so exclude works correctly)
    session.clear_username();

    // Broadcast updated user list
    broadcast_user_list();

    // Notify others
    json notify = {
        {"type", "NOTIFY"},
        {"content", username + " 离开了聊天室"},
        {"timestamp", now_timestamp()}
    };
    if (server_) {
        server_->broadcast(notify);
    }
}

void MessageRouter::handle_broadcast(ClientSession& session, const json& msg) {
    std::string content = msg.value("content", "");
    if (content.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "消息内容不能为空");
        return;
    }

    // Input length validation
    if (content.size() > MAX_MESSAGE_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "消息内容过长（最多4096字符）");
        return;
    }

    int64_t ts = now_timestamp();
    std::string sender = session.get_username();

    // Store to database
    db_.store_message(sender, "__room__", content, ts);

    // Broadcast to all clients
    json broadcast_msg = {
        {"type",      "BROADCAST"},
        {"from",      sender},
        {"content",   content},
        {"timestamp", ts}
    };
    if (server_) {
        server_->broadcast(broadcast_msg);
    }
}

void MessageRouter::handle_private(ClientSession& session, const json& msg) {
    std::string to      = msg.value("to", "");
    std::string content = msg.value("content", "");

    if (to.empty() || content.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少接收方或消息内容");
        return;
    }

    // Input length validation
    if (to.size() > MAX_USERNAME_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "用户名过长（最多32字符）");
        return;
    }
    if (content.size() > MAX_MESSAGE_LEN) {
        Protocol::send_error(session.fd, "INVALID_MSG", "消息内容过长（最多4096字符）");
        return;
    }

    // Use find_session instead of find_fd to prevent fd-reuse race condition.
    // The shared_ptr keeps the target session alive during the send operation.
    auto target_session = server_ ? server_->find_session(to) : nullptr;
    if (!target_session) {
        Protocol::send_error(session.fd, "TARGET_OFFLINE", "目标用户不在线");
        return;
    }

    int64_t ts = now_timestamp();
    std::string sender = session.get_username();

    // Store to database
    db_.store_message(sender, to, content, ts);

    // Send to target
    json private_msg = {
        {"type",      "PRIVATE"},
        {"from",      sender},
        {"to",        to},
        {"content",   content},
        {"timestamp", ts}
    };
    if (server_) {
        server_->send_to_fd(target_session->fd, private_msg);
        // Echo back to sender (so sender can see their own message)
        server_->send_to_fd(session.fd, private_msg);
    }
}

void MessageRouter::handle_history_req(ClientSession& session, const json& msg) {
    std::string to = msg.value("to", "__room__");

    auto history = db_.get_history(to, 50);

    json resp = {
        {"type", "HISTORY_RESP"},
        {"to",   to},
        {"data", history}
    };
    if (server_) {
        server_->send_to_fd(session.fd, resp);
    }
}

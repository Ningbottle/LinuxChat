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

#include <nlohmann/json.hpp>
#include <openssl/evp.h>  // EVP API for SHA-256 (OpenSSL 1.1/3.x)

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

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

void MessageRouter::finish_login(ClientSession& session, const std::string& username) {
    // Set session username (marks as authenticated)
    session.username = username;

    // Add to online users
    {
        std::lock_guard<std::mutex> lock(online_mutex_);
        online_users_.insert(username);
    }

    Protocol::send_ok(session.fd, "LOGIN_OK", username);
    std::cout << "[Handler] User logged in: " << username << "\n";

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

    std::string hash = sha256_hex(password);
    std::cout << "[Auth] register hash for " << username
              << " len=" << hash.size() << std::endl;

    if (!db_.register_user(username, hash)) {
        Protocol::send_error(session.fd, "USER_EXISTS", "用户名已存在");
        return;
    }

    // Registration successful: auto-login
    std::cout << "[Handler] User registered: " << username << "\n";
    finish_login(session, username);
}

void MessageRouter::handle_login(ClientSession& session, const json& msg) {
    std::string username = msg.value("from", "");
    std::string password = msg.value("content", "");

    if (username.empty() || password.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少用户名或密码");
        return;
    }

    // Check if already logged in from another connection
    if (server_ && server_->find_fd(username) >= 0) {
        Protocol::send_error(session.fd, "ALREADY_LOGGED_IN",
                             "该账号已在其他地方登录");
        return;
    }

    std::string hash = sha256_hex(password);
    std::cout << "[Auth] verify hash for " << username
              << " len=" << hash.size() << std::endl;

    if (!db_.verify_user(username, hash)) {
        Protocol::send_error(session.fd, "WRONG_PASSWORD", "密码错误");
        return;
    }

    // Login successful
    finish_login(session, username);
}

void MessageRouter::handle_logout(ClientSession& session) {
    if (!session.is_authenticated()) return;

    std::string username = session.username;

    // Remove from online users
    {
        std::lock_guard<std::mutex> lock(online_mutex_);
        online_users_.erase(username);
    }

    std::cout << "[Handler] User logged out: " << username << "\n";

    // Clear username before broadcast (so exclude works correctly)
    session.username.clear();

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

    int64_t ts = now_timestamp();

    // Store to database
    db_.store_message(session.username, "__room__", content, ts);

    // Broadcast to all clients
    json broadcast_msg = {
        {"type",      "BROADCAST"},
        {"from",      session.username},
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

    int target_fd = server_ ? server_->find_fd(to) : -1;
    if (target_fd < 0) {
        Protocol::send_error(session.fd, "TARGET_OFFLINE", "目标用户不在线");
        return;
    }

    int64_t ts = now_timestamp();

    // Store to database
    db_.store_message(session.username, to, content, ts);

    // Send to target
    json private_msg = {
        {"type",      "PRIVATE"},
        {"from",      session.username},
        {"to",        to},
        {"content",   content},
        {"timestamp", ts}
    };
    if (server_) {
        server_->send_to_fd(target_fd, private_msg);
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

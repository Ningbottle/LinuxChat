// main.cpp — LinuxChat server entry point and message handler
//
// Responsibilities:
//   - SHA-256 password hashing (OpenSSL)
//   - Message handler: routes by "type" field
//   - Disconnect handler: cleanup + notify
//   - Signal handling: SIGINT/SIGTERM → graceful shutdown
//   - Command line: --port, --workers, --db

#include "epoll_server.h"
#include "database.h"
#include "protocol.h"

#include <nlohmann/json.hpp>
#include <openssl/sha.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <csignal>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

using json = nlohmann::json;

// ── Globals ────────────────────────────────────────────────────────

static Database*      g_db     = nullptr;
static EpollServer*   g_server = nullptr;

// ── Helpers ────────────────────────────────────────────────────────

/// SHA-256 hash → lowercase hex string
static std::string sha256_hex(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()),
           input.size(), hash);

    std::ostringstream oss;
    for (unsigned char byte : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte);
    }
    return oss.str();
}

/// Current Unix timestamp in seconds
static int64_t now_timestamp() {
    return static_cast<int64_t>(std::time(nullptr));
}

/// Collect all online usernames.
/// This helper accesses the server's sessions through broadcast trick:
/// We use find_fd() for known usernames. For full list, we need direct access.
/// Since EpollServer doesn't expose sessions_, we track online users in main.

#include <unordered_set>
#include <mutex>

static std::mutex g_online_mutex;
static std::unordered_set<std::string> g_online_users;

static json make_user_list_msg() {
    json users_array = json::array();
    {
        std::lock_guard<std::mutex> lock(g_online_mutex);
        for (const auto& name : g_online_users) {
            users_array.push_back(name);
        }
    }
    return {
        {"type", "USER_LIST"},
        {"data", users_array},
        {"timestamp", now_timestamp()}
    };
}

static void broadcast_user_list() {
    json msg = make_user_list_msg();
    g_server->broadcast(msg);
}

/// Push recent history to a newly logged-in user
static void push_history(int fd, const std::string& username) {
    // Push broadcast (room) history
    auto room_history = g_db->get_history("__room__", 20);
    json hist_msg = {
        {"type", "HISTORY_RESP"},
        {"to",   "__room__"},
        {"data", room_history}
    };
    Protocol::send_msg(fd, hist_msg);
}

// ── Message Handlers ───────────────────────────────────────────────

static void handle_register(ClientSession& session, const json& msg) {
    std::string username = msg.value("from", "");
    std::string password = msg.value("content", "");

    if (username.empty() || password.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少用户名或密码");
        return;
    }

    std::string hash = sha256_hex(password);

    if (!g_db->register_user(username, hash)) {
        Protocol::send_error(session.fd, "USER_EXISTS", "用户名已存在");
        return;
    }

    // Registration successful: auto-login
    session.username = username;

    // Add to online users
    {
        std::lock_guard<std::mutex> lock(g_online_mutex);
        g_online_users.insert(username);
    }

    Protocol::send_ok(session.fd, "LOGIN_OK", username);

    std::cout << "[Handler] User registered: " << username << "\n";

    // Push history + broadcast user list
    push_history(session.fd, username);
    broadcast_user_list();

    // Notify others
    json notify = {
        {"type", "NOTIFY"},
        {"content", username + " 加入了聊天室"},
        {"timestamp", now_timestamp()}
    };
    g_server->broadcast(notify, username);
}

static void handle_login(ClientSession& session, const json& msg) {
    std::string username = msg.value("from", "");
    std::string password = msg.value("content", "");

    if (username.empty() || password.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少用户名或密码");
        return;
    }

    // Check if already logged in from another connection
    if (g_server->find_fd(username) >= 0) {
        Protocol::send_error(session.fd, "ALREADY_LOGGED_IN", "该账号已在其他地方登录");
        return;
    }

    std::string hash = sha256_hex(password);

    if (!g_db->verify_user(username, hash)) {
        Protocol::send_error(session.fd, "WRONG_PASSWORD", "密码错误");
        return;
    }

    // Login successful
    session.username = username;

    // Add to online users
    {
        std::lock_guard<std::mutex> lock(g_online_mutex);
        g_online_users.insert(username);
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
    g_server->broadcast(notify, username);
}

static void handle_logout(ClientSession& session) {
    if (!session.is_authenticated()) return;

    std::string username = session.username;

    // Remove from online users
    {
        std::lock_guard<std::mutex> lock(g_online_mutex);
        g_online_users.erase(username);
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
    g_server->broadcast(notify);
}

static void handle_broadcast(ClientSession& session, const json& msg) {
    std::string content = msg.value("content", "");
    if (content.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "消息内容不能为空");
        return;
    }

    int64_t ts = now_timestamp();

    // Store to database
    g_db->store_message(session.username, "__room__", content, ts);

    // Broadcast to all clients
    json broadcast_msg = {
        {"type",      "BROADCAST"},
        {"from",      session.username},
        {"content",   content},
        {"timestamp", ts}
    };
    g_server->broadcast(broadcast_msg);
}

static void handle_private(ClientSession& session, const json& msg) {
    std::string to      = msg.value("to", "");
    std::string content = msg.value("content", "");

    if (to.empty() || content.empty()) {
        Protocol::send_error(session.fd, "INVALID_MSG", "缺少接收方或消息内容");
        return;
    }

    int target_fd = g_server->find_fd(to);
    if (target_fd < 0) {
        Protocol::send_error(session.fd, "TARGET_OFFLINE", "目标用户不在线");
        return;
    }

    int64_t ts = now_timestamp();

    // Store to database
    g_db->store_message(session.username, to, content, ts);

    // Send to target
    json private_msg = {
        {"type",      "PRIVATE"},
        {"from",      session.username},
        {"to",        to},
        {"content",   content},
        {"timestamp", ts}
    };
    g_server->send_to_fd(target_fd, private_msg);

    // Echo back to sender (so sender can see their own message)
    g_server->send_to_fd(session.fd, private_msg);
}

static void handle_history_req(ClientSession& session, const json& msg) {
    std::string to = msg.value("to", "__room__");

    auto history = g_db->get_history(to, 50);

    json resp = {
        {"type", "HISTORY_RESP"},
        {"to",   to},
        {"data", history}
    };
    Protocol::send_msg(session.fd, resp);
}

// ── Main Message Dispatcher ────────────────────────────────────────

static void handle_message(ClientSession& session, const json& msg) {
    std::string type = msg.value("type", "");

    // Unauthenticated: only REGISTER and LOGIN allowed
    if (!session.is_authenticated()) {
        if (type == "REGISTER") {
            handle_register(session, msg);
        } else if (type == "LOGIN") {
            handle_login(session, msg);
        } else {
            Protocol::send_error(session.fd, "NOT_AUTHENTICATED",
                                 "请先登录或注册");
        }
        return;
    }

    // Authenticated message routing
    if (type == "BROADCAST") {
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

// ── Disconnect Handler ─────────────────────────────────────────────

static void handle_disconnect(ClientSession& session) {
    handle_logout(session);
}

// ── Signal Handler ─────────────────────────────────────────────────

static void signal_handler(int /*sig*/) {
    // Async-signal-safe: use write() instead of std::cout
    const char msg[] = "\n[Server] Shutting down...\n";
    (void)::write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    if (g_server) {
        g_server->stop();
    }
}

// ── Command Line Parsing ───────────────────────────────────────────

struct Config {
    int    port     = 8080;
    int    workers  = 4;
    std::string db_path = "linuxchat.db";
};

static Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            cfg.port = std::atoi(argv[++i]);
        } else if ((arg == "--workers" || arg == "-w") && i + 1 < argc) {
            cfg.workers = std::atoi(argv[++i]);
        } else if ((arg == "--db" || arg == "-d") && i + 1 < argc) {
            cfg.db_path = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: linuxchat_server [options]\n"
                      << "  --port, -p <port>      TCP port (default: 8080)\n"
                      << "  --workers, -w <num>    Worker threads (default: 4)\n"
                      << "  --db, -d <path>        Database path (default: linuxchat.db)\n"
                      << "  --help, -h             Show this help\n";
            exit(0);
        }
    }
    return cfg;
}

// ── Entry Point ────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    Config cfg = parse_args(argc, argv);

    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║        LinuxChat Server v1.0             ║\n"
              << "╠══════════════════════════════════════════╣\n"
              << "║  Port:    " << std::setw(30) << cfg.port     << " ║\n"
              << "║  Workers: " << std::setw(30) << cfg.workers  << " ║\n"
              << "║  DB:      " << std::setw(30) << cfg.db_path  << " ║\n"
              << "╚══════════════════════════════════════════╝\n";

    try {
        Database db(cfg.db_path);
        g_db = &db;

        EpollServer server(cfg.port, static_cast<size_t>(cfg.workers));
        g_server = &server;

        // Register handlers
        server.set_message_handler(handle_message);
        server.set_disconnect_handler(handle_disconnect);

        // Install signal handlers
        signal(SIGINT,  signal_handler);
        signal(SIGTERM, signal_handler);

        // Run (blocks until stop() is called)
        server.run();

    } catch (const std::exception& e) {
        std::cerr << "[Server] Fatal error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Server] Goodbye.\n";
    return 0;
}

// main.cpp — LinuxChat server entry point
//
// Responsibilities:
//   - Command line parsing: --port, --workers, --db
//   - Database + EpollServer + MessageRouter setup
//   - Signal handling: SIGINT/SIGTERM -> graceful shutdown
//
// All message routing logic has been extracted to MessageRouter
// (message_router.h / message_router.cpp).

#include "epoll_server.h"
#include "database.h"
#include "message_router.h"

#include <iostream>
#include <string>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

/// Returns current time as "[YYYY-MM-DD HH:MM:SS.mmm]" for log lines.
static std::string now_stamp() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

// ── Globals (for signal handler only) ───────────────────────────────

static EpollServer* g_server = nullptr;

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
    int         port     = 8080;
    int         workers  = 4;
    std::string db_path  = "linuxchat.db";
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
              << "║  Port:    " << std::setw(30) << cfg.port    << " ║\n"
              << "║  Workers: " << std::setw(30) << cfg.workers << " ║\n"
              << "║  DB:      " << std::setw(30) << cfg.db_path << " ║\n"
              << "╚══════════════════════════════════════════╝\n";

    try {
        Database db(cfg.db_path);
        EpollServer server(cfg.port, static_cast<size_t>(cfg.workers));
        MessageRouter router(db);

        g_server = &server;
        router.set_server(&server);

        // Register handlers: route through MessageRouter
        server.set_message_handler(
            [&router](ClientSession& session, const nlohmann::json& msg) {
                router.route(session, msg);
            });
        server.set_disconnect_handler(
            [&router](ClientSession& session) {
                router.handle_logout(session);
            });

        // Install signal handlers
        signal(SIGINT,  signal_handler);
        signal(SIGTERM, signal_handler);

        // Run (blocks until stop() is called)
        server.run();

    } catch (const std::exception& e) {
        std::cerr << now_stamp() << " [Server] Fatal error: " << e.what() << "\n";
        return 1;
    }

    std::cout << now_stamp() << " [Server] Goodbye.\n";
    return 0;
}

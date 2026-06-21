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
#include "log_utils.h"

#include <string>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

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
    int         port     = 18080;
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
            fmt::print("Usage: linuxchat_server [options]\n"
                       "  --port, -p <port>      TCP port (default: 18080)\n"
                       "  --workers, -w <num>    Worker threads (default: 4)\n"
                       "  --db, -d <path>        Database path (default: linuxchat.db)\n"
                       "  --help, -h             Show this help\n");
            exit(0);
        }
    }
    return cfg;
}

// ── Entry Point ────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    init_logger();
    Config cfg = parse_args(argc, argv);

    spdlog::info("╔══════════════════════════════════════════╗");
    spdlog::info("║        LinuxChat Server v1.0             ║");
    spdlog::info("╠══════════════════════════════════════════╣");
    spdlog::info("║  Port:    {:>30} ║", cfg.port);
    spdlog::info("║  Workers: {:>30} ║", cfg.workers);
    spdlog::info("║  DB:      {:>30} ║", cfg.db_path);
    spdlog::info("╚══════════════════════════════════════════╝");

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
                // Clean up login reservation if client disconnects mid-login
                std::string pending = session.get_pending_login();
                if (!pending.empty()) {
                    router.cleanup_login_reservation(pending);
                    session.clear_pending_login();
                }
                router.handle_logout(session);
            });

        // Install signal handlers (sigaction is POSIX-guaranteed, unlike signal())
        struct sigaction sa{};
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;  // No SA_RESTART — we want epoll_wait to return EINTR
        sigaction(SIGINT,  &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);

        // Run (blocks until stop() is called)
        server.run();

    } catch (const std::exception& e) {
        spdlog::error("[Server] Fatal error: {}", e.what());
        return 1;
    }

    spdlog::info("[Server] Goodbye.");
    return 0;
}

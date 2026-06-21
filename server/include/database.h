#pragma once
// database.h — SQLite3 persistence layer for LinuxChat server
//
// Thread-safe: all public methods are mutex-protected.
// Uses WAL mode and prepared statements for performance.
// Password hashes (not plaintext) are stored in the users table.

#include <string>
#include <vector>
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

class Database {
public:
    /// Opens (or creates) the SQLite database at `db_path`.
    /// Use ":memory:" for in-memory testing.
    /// Automatically initializes schema and enables WAL mode.
    explicit Database(const std::string& db_path = "linuxchat.db");

    /// Closes the database connection.
    ~Database();

    // Non-copyable, non-movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // ── User Management ──────────────────────────────────────────

    /// Register a new user. `password_hash` should be "salt_hex:hash_hex" format.
    /// @return true if user was created; false if username already exists or is empty.
    bool register_user(const std::string& username, const std::string& password_hash);

    /// Get the stored password hash for a user (for salted verification).
    /// @return stored hash string, or empty string if user not found.
    std::string get_stored_hash(const std::string& username);

    // ── Message Storage ──────────────────────────────────────────

    /// Store a chat message.
    /// @param to  "__room__" for broadcast; username for private messages.
    /// @param timestamp  Unix timestamp in seconds.
    void store_message(const std::string& from, const std::string& to,
                       const std::string& content, int64_t timestamp);

    /// Retrieve message history for a target.
    /// @param requestor The user requesting the history.
    /// @param target    "__room__" for broadcast history; username for private history.
    /// @param limit     Maximum number of messages to return (default 20, max 200).
    /// @return Vector of JSON objects: {"from": "...", "content": "...", "timestamp": N}
    std::vector<nlohmann::json> get_history(const std::string& requestor, const std::string& target, int limit = 20);

private:
    void init_schema();
    void exec_sql(const std::string& sql);
    void prepare_statements();

    sqlite3* db_ = nullptr;
    mutable std::shared_mutex db_mutex_;

    sqlite3_stmt* stmt_register_ = nullptr;
    sqlite3_stmt* stmt_get_hash_ = nullptr;
    sqlite3_stmt* stmt_store_msg_ = nullptr;
    sqlite3_stmt* stmt_hist_room_ = nullptr;
    sqlite3_stmt* stmt_hist_priv_ = nullptr;
};

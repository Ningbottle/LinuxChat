// database.cpp — SQLite3 persistence layer implementation
//
// All public methods are mutex-protected for thread safety.
// Uses prepared statements to prevent SQL injection.

#include "database.h"
#include <stdexcept>
#include <algorithm>
#include <spdlog/spdlog.h>

// ── Constructor / Destructor ───────────────────────────────────────

Database::Database(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Database: failed to open: " + err);
    }

    // Enable WAL mode (skip for :memory: — not supported)
    if (db_path != ":memory:") {
        exec_sql("PRAGMA journal_mode=WAL;");
    }

    // Performance pragmas
    exec_sql("PRAGMA synchronous=NORMAL;");
    exec_sql("PRAGMA busy_timeout=5000;");
    exec_sql("PRAGMA temp_store=MEMORY;");
    exec_sql("PRAGMA cache_size=-64000;"); // 64 MB

    init_schema();
    prepare_statements();
}

Database::~Database() {
    if (db_) {
        sqlite3_finalize(stmt_register_);
        sqlite3_finalize(stmt_verify_);
        sqlite3_finalize(stmt_get_hash_);
        sqlite3_finalize(stmt_store_msg_);
        sqlite3_finalize(stmt_hist_room_);
        sqlite3_finalize(stmt_hist_priv_);

        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// ── Private Helpers ────────────────────────────────────────────────

void Database::exec_sql(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "unknown error";
        sqlite3_free(err_msg);
        throw std::runtime_error("Database::exec_sql: " + err + " | SQL: " + sql);
    }
}

void Database::init_schema() {
    exec_sql(R"(
        CREATE TABLE IF NOT EXISTS users (
            username     TEXT PRIMARY KEY,
            password_hash TEXT NOT NULL,
            created_at   INTEGER DEFAULT (strftime('%s','now'))
        );
    )");

    exec_sql(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            from_user  TEXT    NOT NULL,
            to_user    TEXT    NOT NULL,
            content    TEXT    NOT NULL,
            timestamp  INTEGER NOT NULL
        );
    )");

    exec_sql(R"(
        CREATE INDEX IF NOT EXISTS idx_messages_to_ts
            ON messages(to_user, timestamp);
    )");
}

void Database::prepare_statements() {
    auto prepare = [this](const char* sql, sqlite3_stmt** stmt) {
        if (sqlite3_prepare_v2(db_, sql, -1, stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Database failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        }
    };

    prepare("INSERT OR IGNORE INTO users (username, password_hash) VALUES (?, ?);", &stmt_register_);
    prepare("SELECT password_hash FROM users WHERE username = ?;", &stmt_verify_);
    prepare("SELECT password_hash FROM users WHERE username = ?;", &stmt_get_hash_);
    prepare("INSERT INTO messages (from_user, to_user, content, timestamp) VALUES (?, ?, ?, ?);", &stmt_store_msg_);
    
    prepare("SELECT * FROM ("
            "SELECT from_user, content, timestamp FROM messages "
            "WHERE to_user = ? ORDER BY timestamp DESC LIMIT ?"
            ") ORDER BY timestamp ASC;", &stmt_hist_room_);

    prepare("SELECT * FROM ("
            "SELECT from_user, content, timestamp FROM messages "
            "WHERE (from_user = ? AND to_user = ?) OR (from_user = ? AND to_user = ?) "
            "ORDER BY timestamp DESC LIMIT ?"
            ") ORDER BY timestamp ASC;", &stmt_hist_priv_);
}

// ── User Management ────────────────────────────────────────────────

bool Database::register_user(const std::string& username, const std::string& password_hash) {
    if (username.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(db_mutex_);

    sqlite3_reset(stmt_register_);
    sqlite3_clear_bindings(stmt_register_);

    sqlite3_bind_text(stmt_register_, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_register_, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt_register_);

    if (rc != SQLITE_DONE) {
        spdlog::error("[Database] register_user step error: {}", sqlite3_errmsg(db_));
        return false;
    }

    // INSERT OR IGNORE: if row was ignored (duplicate), changes() == 0
    return sqlite3_changes(db_) > 0;
}

bool Database::verify_user(const std::string& username, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    sqlite3_reset(stmt_verify_);
    sqlite3_clear_bindings(stmt_verify_);

    sqlite3_bind_text(stmt_verify_, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt_verify_);
    bool result = false;

    if (rc == SQLITE_ROW) {
        const char* stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt_verify_, 0));
        if (stored_hash && password_hash == stored_hash) {
            result = true;
        }
    }

    return result;
}

std::string Database::get_stored_hash(const std::string& username) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    sqlite3_reset(stmt_get_hash_);
    sqlite3_clear_bindings(stmt_get_hash_);

    sqlite3_bind_text(stmt_get_hash_, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::string stored;
    if (sqlite3_step(stmt_get_hash_) == SQLITE_ROW) {
        const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get_hash_, 0));
        if (hash) stored = hash;
    }

    return stored;
}

// ── Message Storage ────────────────────────────────────────────────

void Database::store_message(const std::string& from, const std::string& to,
                             const std::string& content, int64_t timestamp) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    sqlite3_reset(stmt_store_msg_);
    sqlite3_clear_bindings(stmt_store_msg_);

    sqlite3_bind_text(stmt_store_msg_, 1, from.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_store_msg_, 2, to.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_store_msg_, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt_store_msg_, 4, timestamp);

    int rc = sqlite3_step(stmt_store_msg_);

    if (rc != SQLITE_DONE) {
        spdlog::error("[Database] store_message error: {}", sqlite3_errmsg(db_));
    }
}

std::vector<nlohmann::json> Database::get_history(const std::string& requestor, const std::string& target, int limit) {
    // Clamp limit to [1, 200]
    limit = std::clamp(limit, 1, 200);

    std::lock_guard<std::mutex> lock(db_mutex_);

    sqlite3_stmt* stmt = nullptr;
    
    if (target == "__room__") {
        stmt = stmt_hist_room_;
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, target.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, limit);
    } else {
        stmt = stmt_hist_priv_;
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, requestor.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, target.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, target.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, requestor.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, limit);
    }

    std::vector<nlohmann::json> result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* from_user = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* content   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int64_t ts            = sqlite3_column_int64(stmt, 2);

        result.push_back({
            {"from",      from_user ? from_user : ""},
            {"content",   content   ? content   : ""},
            {"timestamp", ts}
        });
    }

    return result;
}

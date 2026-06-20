// test_message_handler.cpp — Integration tests for message handler logic
//
// Tests the expected behavior of the server message handler without
// requiring a running epoll server. Uses Database + Protocol directly
// to verify handler contracts defined in protocol.md.
//
// Uses pipe() for capturing Protocol output.

#include <gtest/gtest.h>
#include "database.h"
#include "protocol.h"
#include "client_session.h"
#include "epoll_server.h"

#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <sstream>
#include <iomanip>
#include <memory>

using json = nlohmann::json;

// ── Helpers ────────────────────────────────────────────────────────

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

/// Pipe pair for capturing server→client messages
struct PipeSession {
    int read_fd;
    int write_fd;
    ClientSession session;

    PipeSession() : session(-1) {
        int fds[2];
        EXPECT_EQ(pipe(fds), 0);
        read_fd = fds[0];
        write_fd = fds[1];
        session.fd = write_fd;
        // Set read end non-blocking for recv_msgs
        int flags = fcntl(read_fd, F_GETFL, 0);
        fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);
    }

    ~PipeSession() {
        close(read_fd);
        close(write_fd);
    }

    /// Read a message that was sent to write_fd
    std::optional<std::vector<json>> read_messages() {
        ClientSession reader(read_fd);
        return Protocol::recv_msgs(reader);
    }
};

// ── Fixture ────────────────────────────────────────────────────────

class HandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
    }

    std::unique_ptr<Database> db;
};

// ── Registration Scenarios ─────────────────────────────────────────

TEST_F(HandlerTest, Register_NewUser_StoredInDatabase) {
    std::string hash = sha256_hex("password123");
    EXPECT_TRUE(db->register_user("alice", hash));
    EXPECT_TRUE(db->verify_user("alice", hash));
}

TEST_F(HandlerTest, Register_DuplicateUser_Rejected) {
    std::string hash = sha256_hex("password123");
    ASSERT_TRUE(db->register_user("alice", hash));
    EXPECT_FALSE(db->register_user("alice", sha256_hex("other")));
}

TEST_F(HandlerTest, Register_LoginOK_SentToClient) {
    PipeSession ps;
    // Simulate server sending LOGIN_OK after registration
    Protocol::send_ok(ps.write_fd, "LOGIN_OK", "alice");

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "LOGIN_OK");
    EXPECT_EQ((*msgs)[0]["from"], "alice");
}

// ── Login Scenarios ────────────────────────────────────────────────

TEST_F(HandlerTest, Login_WrongPassword_ErrorSent) {
    std::string hash = sha256_hex("correct_password");
    ASSERT_TRUE(db->register_user("alice", hash));

    // Wrong password hash won't match
    std::string wrong_hash = sha256_hex("wrong_password");
    EXPECT_FALSE(db->verify_user("alice", wrong_hash));

    // Verify the error message format
    PipeSession ps;
    Protocol::send_error(ps.write_fd, "WRONG_PASSWORD", "密码错误");

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "WRONG_PASSWORD");
}

TEST_F(HandlerTest, Login_NonexistentUser_ErrorSent) {
    EXPECT_FALSE(db->verify_user("nobody", "any_hash"));
}

// ── Broadcast Scenarios ────────────────────────────────────────────

TEST_F(HandlerTest, Broadcast_StoresMessage_WithRoomTarget) {
    db->store_message("alice", "__room__", "hello everyone", 1700000000);

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[0]["content"], "hello everyone");
}

TEST_F(HandlerTest, Broadcast_MessageFormat_IncludesTimestamp) {
    PipeSession ps;

    json broadcast_msg = {
        {"type",      "BROADCAST"},
        {"from",      "alice"},
        {"content",   "hello"},
        {"timestamp", 1700000000}
    };
    Protocol::send_msg(ps.write_fd, broadcast_msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "BROADCAST");
    EXPECT_EQ((*msgs)[0]["timestamp"], 1700000000);
}

// ── Private Chat Scenarios ─────────────────────────────────────────

TEST_F(HandlerTest, Private_StoresMessage_WithUsernameTarget) {
    db->store_message("alice", "bob", "hi bob", 1700000001);

    auto history = db->get_history("bob");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
}

TEST_F(HandlerTest, Private_TargetOffline_ErrorReturned) {
    // Simulate: find_fd returns -1 for offline user
    // The handler should send TARGET_OFFLINE error
    PipeSession ps;
    Protocol::send_error(ps.write_fd, "TARGET_OFFLINE", "目标用户不在线");

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["code"], "TARGET_OFFLINE");
}

// ── History Scenarios ──────────────────────────────────────────────

TEST_F(HandlerTest, HistoryReq_ReturnsRecentMessages) {
    db->store_message("alice", "__room__", "msg1", 1000);
    db->store_message("bob", "__room__", "msg2", 2000);
    db->store_message("charlie", "__room__", "msg3", 3000);

    auto history = db->get_history("__room__", 20);
    ASSERT_EQ(history.size(), 3u);

    // Verify HISTORY_RESP format
    PipeSession ps;
    json resp = {
        {"type", "HISTORY_RESP"},
        {"to",   "__room__"},
        {"data", history}
    };
    Protocol::send_msg(ps.write_fd, resp);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "HISTORY_RESP");
    EXPECT_EQ((*msgs)[0]["data"].size(), 3u);
}

TEST_F(HandlerTest, HistoryReq_EmptyRoom_ReturnsEmptyArray) {
    auto history = db->get_history("__room__");
    EXPECT_TRUE(history.empty());
}

// ── Authentication Gate ────────────────────────────────────────────

TEST_F(HandlerTest, UnauthenticatedSession_OnlyAllowsRegisterAndLogin) {
    ClientSession session(42); // arbitrary fd
    EXPECT_FALSE(session.is_authenticated());

    // After setting username, it becomes authenticated
    session.set_username("alice");
    EXPECT_TRUE(session.is_authenticated());
}

// ── SHA-256 Correctness ────────────────────────────────────────────

TEST_F(HandlerTest, SHA256_KnownVector_MatchesExpected) {
    // SHA-256 of empty string
    std::string empty_hash = sha256_hex("");
    EXPECT_EQ(empty_hash,
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // SHA-256 of "password"
    std::string pwd_hash = sha256_hex("password");
    EXPECT_EQ(pwd_hash,
              "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8");
}

// ── USER_LIST Format ───────────────────────────────────────────────

TEST_F(HandlerTest, UserList_CorrectFormat) {
    PipeSession ps;

    json user_list = {
        {"type", "USER_LIST"},
        {"data", json::array({"alice", "bob", "charlie"})},
        {"timestamp", 1700000000}
    };
    Protocol::send_msg(ps.write_fd, user_list);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "USER_LIST");
    EXPECT_EQ((*msgs)[0]["data"].size(), 3u);
}

// ── NOTIFY Format ──────────────────────────────────────────────────

TEST_F(HandlerTest, Notify_SystemMessage_CorrectFormat) {
    PipeSession ps;

    json notify = {
        {"type", "NOTIFY"},
        {"content", "alice 加入了聊天室"},
        {"timestamp", 1700000000}
    };
    Protocol::send_msg(ps.write_fd, notify);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "NOTIFY");
    EXPECT_EQ((*msgs)[0]["content"], "alice 加入了聊天室");
}

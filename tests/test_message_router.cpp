// test_message_router.cpp — Unit tests for MessageRouter
//
// Tests message routing logic without a running EpollServer.
// Uses Database(":memory:") for isolation and pipe()-based PipeSession
// to capture Protocol messages sent to the client fd.
//
// Since server_ is null, operations like broadcast/push_history are skipped.
// This isolates the routing and database logic for unit testing.

#include <gtest/gtest.h>
#include "message_router.h"
#include "database.h"
#include "protocol.h"
#include "client_session.h"

#include <nlohmann/json.hpp>
#include <unistd.h>
#include <fcntl.h>

using json = nlohmann::json;

// ── Helpers ────────────────────────────────────────────────────────

/// Pipe pair that captures Protocol messages sent to write_fd.
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

    /// Read messages that were sent to write_fd via Protocol::send_msg
    std::optional<std::vector<json>> read_messages() {
        ClientSession reader(read_fd);
        return Protocol::recv_msgs(reader);
    }
};

// ── Fixture ────────────────────────────────────────────────────────

class MessageRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
        router = std::make_unique<MessageRouter>(*db);
        // server_ is null by default — broadcast/find_fd/push_history are no-ops
    }

    std::unique_ptr<Database> db;
    std::unique_ptr<MessageRouter> router;
};

// ── Registration Tests ─────────────────────────────────────────────

TEST_F(MessageRouterTest, Register_Success_SendsLoginOk) {
    PipeSession ps;
    json msg = {{"type", "REGISTER"}, {"from", "testuser"}, {"content", "pass123"}};
    router->handle_register(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "LOGIN_OK");
    EXPECT_EQ((*msgs)[0]["from"], "testuser");

    // Verify user was stored in database
    EXPECT_TRUE(db->verify_user("testuser",
        MessageRouter::sha256_hex("pass123")));
}

TEST_F(MessageRouterTest, Register_DuplicateUser_SendsError) {
    PipeSession ps1, ps2;
    json msg = {{"type", "REGISTER"}, {"from", "testuser"}, {"content", "pass123"}};

    router->handle_register(ps1.session, msg);
    // Drain first response
    ps1.read_messages();

    router->handle_register(ps2.session, msg);
    auto msgs = ps2.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "USER_EXISTS");
}

TEST_F(MessageRouterTest, Register_EmptyUsername_SendsError) {
    PipeSession ps;
    json msg = {{"type", "REGISTER"}, {"from", ""}, {"content", "pass123"}};
    router->handle_register(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

TEST_F(MessageRouterTest, Register_EmptyPassword_SendsError) {
    PipeSession ps;
    json msg = {{"type", "REGISTER"}, {"from", "testuser"}, {"content", ""}};
    router->handle_register(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

// ── Login Tests ────────────────────────────────────────────────────

TEST_F(MessageRouterTest, Login_Success_SendsLoginOk) {
    // Register first
    db->register_user("testuser", MessageRouter::sha256_hex("pass123"));

    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "testuser"}, {"content", "pass123"}};
    router->handle_login(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "LOGIN_OK");
    EXPECT_EQ((*msgs)[0]["from"], "testuser");
}

TEST_F(MessageRouterTest, Login_WrongPassword_SendsError) {
    db->register_user("testuser", MessageRouter::sha256_hex("pass123"));

    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "testuser"}, {"content", "wrong"}};
    router->handle_login(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "WRONG_PASSWORD");
}

TEST_F(MessageRouterTest, Login_NonexistentUser_SendsError) {
    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "nobody"}, {"content", "pass"}};
    router->handle_login(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "WRONG_PASSWORD");
}

TEST_F(MessageRouterTest, Login_EmptyFields_SendsError) {
    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", ""}, {"content", ""}};
    router->handle_login(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

// ── Session Authentication ─────────────────────────────────────────

TEST_F(MessageRouterTest, Session_InitiallyNotAuthenticated) {
    ClientSession session(42);
    EXPECT_FALSE(session.is_authenticated());
    EXPECT_TRUE(session.get_username().empty());
}

TEST_F(MessageRouterTest, Session_AuthenticatedAfterLogin) {
    db->register_user("alice", MessageRouter::sha256_hex("pass"));

    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "alice"}, {"content", "pass"}};
    router->handle_login(ps.session, msg);

    // After successful login, session.username should be set
    EXPECT_TRUE(ps.session.is_authenticated());
    EXPECT_EQ(ps.session.get_username(), "alice");
}

// ── Broadcast Tests ────────────────────────────────────────────────

TEST_F(MessageRouterTest, Broadcast_StoresMessageInDatabase) {
    PipeSession ps;
    ps.session.set_username("alice");  // Authenticate

    json msg = {{"type", "BROADCAST"}, {"content", "hello everyone"}};
    router->handle_broadcast(ps.session, msg);

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[0]["content"], "hello everyone");
}

TEST_F(MessageRouterTest, Broadcast_EmptyContent_SendsError) {
    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "BROADCAST"}, {"content", ""}};
    router->handle_broadcast(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

// ── Private Message Tests ──────────────────────────────────────────

TEST_F(MessageRouterTest, Private_TargetOffline_SendsError) {
    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "PRIVATE"}, {"to", "bob"}, {"content", "hi"}};
    router->handle_private(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "TARGET_OFFLINE");
}

TEST_F(MessageRouterTest, Private_EmptyTo_SendsError) {
    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "PRIVATE"}, {"to", ""}, {"content", "hi"}};
    router->handle_private(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

TEST_F(MessageRouterTest, Private_EmptyContent_SendsError) {
    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "PRIVATE"}, {"to", "bob"}, {"content", ""}};
    router->handle_private(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

// ── History Request Tests ──────────────────────────────────────────

TEST_F(MessageRouterTest, HistoryReq_ReturnsStoredMessages) {
    // Store some messages
    db->store_message("alice", "__room__", "msg1", 1000);
    db->store_message("bob", "__room__", "msg2", 2000);

    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "HISTORY_REQ"}, {"to", "__room__"}};
    router->handle_history_req(ps.session, msg);

    // Note: with null server, the response is not sent via send_to_fd.
    // But we can verify the database has the messages.
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 2u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[1]["from"], "bob");
}

// ── Route Dispatcher Tests ─────────────────────────────────────────

TEST_F(MessageRouterTest, Route_Unauthenticated_BroadcastRejected) {
    PipeSession ps;
    // session is not authenticated (username empty)

    json msg = {{"type", "BROADCAST"}, {"content", "hello"}};
    router->route(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "NOT_AUTHENTICATED");
}

TEST_F(MessageRouterTest, Route_Unauthenticated_UnknownTypeRejected) {
    PipeSession ps;

    json msg = {{"type", "UNKNOWN_TYPE"}, {"content", "data"}};
    router->route(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "NOT_AUTHENTICATED");
}

TEST_F(MessageRouterTest, Route_Authenticated_UnknownTypeRejected) {
    PipeSession ps;
    ps.session.set_username("alice");

    json msg = {{"type", "UNKNOWN_TYPE"}, {"content", "data"}};
    router->route(ps.session, msg);

    auto msgs = ps.read_messages();
    ASSERT_TRUE(msgs.has_value());
    ASSERT_GE(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "INVALID_MSG");
}

// ── Logout Tests ───────────────────────────────────────────────────

TEST_F(MessageRouterTest, Logout_ClearsSession) {
    PipeSession ps;
    ps.session.set_username("alice");

    router->handle_logout(ps.session);

    EXPECT_FALSE(ps.session.is_authenticated());
    EXPECT_TRUE(ps.session.get_username().empty());
}

TEST_F(MessageRouterTest, Logout_UnauthenticatedSession_NoOp) {
    PipeSession ps;
    // session.username is empty

    router->handle_logout(ps.session);

    // Should not crash or send any messages
    EXPECT_FALSE(ps.session.is_authenticated());
}

// ── Online User Tracking ───────────────────────────────────────────

TEST_F(MessageRouterTest, OnlineUser_TrackedAfterRegister) {
    PipeSession ps;
    json msg = {{"type", "REGISTER"}, {"from", "alice"}, {"content", "pass"}};
    router->handle_register(ps.session, msg);

    EXPECT_TRUE(router->is_user_online("alice"));
}

TEST_F(MessageRouterTest, OnlineUser_TrackedAfterLogin) {
    db->register_user("alice", MessageRouter::sha256_hex("pass"));

    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "alice"}, {"content", "pass"}};
    router->handle_login(ps.session, msg);

    EXPECT_TRUE(router->is_user_online("alice"));
}

TEST_F(MessageRouterTest, OnlineUser_RemovedAfterLogout) {
    db->register_user("alice", MessageRouter::sha256_hex("pass"));

    PipeSession ps;
    json msg = {{"type", "LOGIN"}, {"from", "alice"}, {"content", "pass"}};
    router->handle_login(ps.session, msg);
    EXPECT_TRUE(router->is_user_online("alice"));

    router->handle_logout(ps.session);
    EXPECT_FALSE(router->is_user_online("alice"));
}

TEST_F(MessageRouterTest, OnlineUser_NotTrackedBeforeLogin) {
    EXPECT_FALSE(router->is_user_online("nobody"));
}

// ── Full Flow Tests ────────────────────────────────────────────────

TEST_F(MessageRouterTest, FullFlow_Register_Login_Chat) {
    // Register alice
    PipeSession ps_alice;
    json reg_msg = {{"type", "REGISTER"}, {"from", "alice"}, {"content", "pass123"}};
    router->handle_register(ps_alice.session, reg_msg);
    auto reg_resp = ps_alice.read_messages();
    ASSERT_TRUE(reg_resp.has_value());
    EXPECT_EQ((*reg_resp)[0]["type"], "LOGIN_OK");

    // Register bob
    PipeSession ps_bob;
    json reg_msg2 = {{"type", "REGISTER"}, {"from", "bob"}, {"content", "pass456"}};
    router->handle_register(ps_bob.session, reg_msg2);

    // Alice sends broadcast
    json bcast = {{"type", "BROADCAST"}, {"content", "hello everyone"}};
    router->handle_broadcast(ps_alice.session, bcast);

    // Verify in database
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[0]["content"], "hello everyone");

    // Both online
    EXPECT_TRUE(router->is_user_online("alice"));
    EXPECT_TRUE(router->is_user_online("bob"));
}

TEST_F(MessageRouterTest, FullFlow_MultipleBroadcasts) {
    PipeSession ps;
    ps.session.set_username("alice");

    for (int i = 0; i < 5; ++i) {
        json msg = {{"type", "BROADCAST"}, {"content", "msg" + std::to_string(i)}};
        router->handle_broadcast(ps.session, msg);
    }

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 5u);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(history[i]["content"], "msg" + std::to_string(i));
    }
}

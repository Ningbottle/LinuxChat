// test_protocol.cpp — Unit tests for JSON-over-TCP framing (protocol.cpp)
//
// Uses pipe() to create a valid fd pair for testing recv_msgs() end-to-end.
// Only compiles on Linux (matches server target platform).

#include <gtest/gtest.h>
#include "protocol.h"
#include "client_session.h"
#include <nlohmann/json.hpp>
#include <unistd.h>     // pipe(), write(), close()
#include <arpa/inet.h>  // htonl()
#include <fcntl.h>
#include <cstring>

using json = nlohmann::json;

// ── Helpers ────────────────────────────────────────────────────────

/// Create a pipe pair. Returns {read_fd, write_fd}.
static std::pair<int, int> make_pipe() {
    int fds[2];
    EXPECT_EQ(pipe(fds), 0);
    // Set read end to non-blocking (recv_msgs expects non-blocking fd)
    int flags = fcntl(fds[0], F_GETFL, 0);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
    return {fds[0], fds[1]};
}

/// Write a framed message (4-byte BE length + JSON body) to write_fd.
static void write_frame(int write_fd, const json& msg) {
    std::string body = msg.dump();
    uint32_t net_len = htonl(static_cast<uint32_t>(body.size()));
    ASSERT_EQ(write(write_fd, &net_len, 4), 4);
    ASSERT_EQ(write(write_fd, body.c_str(), body.size()),
              static_cast<ssize_t>(body.size()));
}

/// Write raw bytes to write_fd (for partial/malformed frame tests).
static void write_raw(int write_fd, const void* data, size_t len) {
    ASSERT_EQ(write(write_fd, data, len), static_cast<ssize_t>(len));
}

// ── Tests ──────────────────────────────────────────────────────────

TEST(ProtocolSend, ValidJson_ReturnsTrue) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {{"type", "BROADCAST"}, {"from", "alice"}, {"content", "hello"}};
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    // Verify: read back and check
    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "BROADCAST");
    EXPECT_EQ((*msgs)[0]["from"], "alice");
    EXPECT_EQ((*msgs)[0]["content"], "hello");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSend, MultipleMessages_AllReceived) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Send 3 messages
    for (int i = 0; i < 3; ++i) {
        json msg = {{"type", "BROADCAST"}, {"content", std::to_string(i)}};
        EXPECT_TRUE(Protocol::send_msg(write_fd, msg));
    }

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 3u);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ((*msgs)[i]["content"], std::to_string(i));
    }

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, IncompleteHeader_ReturnsEmpty) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Write only 2 bytes (incomplete 4-byte header)
    uint8_t partial[] = {0x00, 0x00};
    write_raw(write_fd, partial, 2);

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    EXPECT_TRUE(msgs->empty()); // No complete frames yet

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, IncompleteBody_ReturnsEmpty) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Write header claiming 100 bytes, but only send 10 bytes of body
    uint32_t net_len = htonl(100);
    write_raw(write_fd, &net_len, 4);

    char partial_body[10] = {"hello"};
    write_raw(write_fd, partial_body, 10);

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    EXPECT_TRUE(msgs->empty()); // Body incomplete

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, StickyPacket_TwoFramesInOneRead) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Send two messages back-to-back (they'll arrive as one chunk)
    json msg1 = {{"type", "BROADCAST"}, {"content", "first"}};
    json msg2 = {{"type", "BROADCAST"}, {"content", "second"}};
    write_frame(write_fd, msg1);
    write_frame(write_fd, msg2);

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 2u);
    EXPECT_EQ((*msgs)[0]["content"], "first");
    EXPECT_EQ((*msgs)[1]["content"], "second");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, HalfPacket_SecondReadCompletesFrame) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {{"type", "BROADCAST"}, {"content", "hello world"}};
    std::string body = msg.dump();
    uint32_t net_len = htonl(static_cast<uint32_t>(body.size()));

    // Write header + half of body
    size_t half = body.size() / 2;
    write_raw(write_fd, &net_len, 4);
    write_raw(write_fd, body.c_str(), half);

    // First read: incomplete
    auto msgs1 = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs1.has_value());
    EXPECT_TRUE(msgs1->empty());

    // Write remaining half
    write_raw(write_fd, body.c_str() + half, body.size() - half);

    // Second read: complete
    auto msgs2 = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs2.has_value());
    ASSERT_EQ(msgs2->size(), 1u);
    EXPECT_EQ((*msgs2)[0]["content"], "hello world");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, OversizedFrame_ReturnsNullopt) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Send a header claiming > 16MB body
    uint32_t net_len = htonl(17 * 1024 * 1024); // 17 MB
    write_raw(write_fd, &net_len, 4);

    auto msgs = Protocol::recv_msgs(session);
    EXPECT_FALSE(msgs.has_value()); // Should reject oversized frame

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, MalformedJson_SkipsInvalid) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Send valid frame first
    json valid = {{"type", "BROADCAST"}, {"content", "ok"}};
    write_frame(write_fd, valid);

    // Send a frame with invalid JSON body
    std::string bad_body = "this is not json{{{";
    uint32_t bad_len = htonl(static_cast<uint32_t>(bad_body.size()));
    write_raw(write_fd, &bad_len, 4);
    write_raw(write_fd, bad_body.c_str(), bad_body.size());

    // Send another valid frame
    json valid2 = {{"type", "BROADCAST"}, {"content", "also ok"}};
    write_frame(write_fd, valid2);

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    // Should get 2 valid messages (the malformed one is skipped)
    ASSERT_EQ(msgs->size(), 2u);
    EXPECT_EQ((*msgs)[0]["content"], "ok");
    EXPECT_EQ((*msgs)[1]["content"], "also ok");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolRecv, ConnectionClosed_ReturnsNullopt) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Close write end to simulate peer disconnect
    close(write_fd);

    auto msgs = Protocol::recv_msgs(session);
    EXPECT_FALSE(msgs.has_value()); // recv() returns 0 → nullopt

    close(read_fd);
}

TEST(ProtocolRecv, EmptyRead_NoDataAvailable) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // Don't write anything; fd is non-blocking → EAGAIN
    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    EXPECT_TRUE(msgs->empty());

    close(read_fd);
    close(write_fd);
}

// ── Content Tests ──────────────────────────────────────────────────

TEST(ProtocolSend, LargeMessage_10KB_Content) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // 10KB content (fits in pipe buffer; avoids write-block deadlock)
    std::string large_content(10 * 1024, 'x');
    json msg = {{"type", "BROADCAST"}, {"from", "alice"}, {"content", large_content}};
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["content"], large_content);

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSend, EmptyContent) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {{"type", "BROADCAST"}, {"from", "alice"}, {"content", ""}};
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["content"], "");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSend, UnicodeContent) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {{"type", "BROADCAST"}, {"from", "alice"}, {"content", "你好世界🌍"}};
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["content"], "你好世界🌍");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSend, MultipleFields) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {
        {"type",      "PRIVATE"},
        {"from",      "alice"},
        {"to",        "bob"},
        {"content",   "secret message"},
        {"timestamp", 1700000000}
    };
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "PRIVATE");
    EXPECT_EQ((*msgs)[0]["from"], "alice");
    EXPECT_EQ((*msgs)[0]["to"], "bob");
    EXPECT_EQ((*msgs)[0]["content"], "secret message");
    EXPECT_EQ((*msgs)[0]["timestamp"], 1700000000);

    close(read_fd);
    close(write_fd);
}

// ── Send Error / OK Edge Cases ─────────────────────────────────────

TEST(ProtocolSendError, SendsErrorFrame) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    EXPECT_TRUE(Protocol::send_error(write_fd, "WRONG_PASSWORD", "密码错误"));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "WRONG_PASSWORD");
    EXPECT_EQ((*msgs)[0]["content"], "密码错误");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSendOk, SendsOkFrame) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    EXPECT_TRUE(Protocol::send_ok(write_fd, "LOGIN_OK", "alice"));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "LOGIN_OK");
    EXPECT_EQ((*msgs)[0]["from"], "alice");

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSendError, EmptyContent_UsesCodeAsContent) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    // When content is empty, Protocol uses code as content
    EXPECT_TRUE(Protocol::send_error(write_fd, "WRONG_PASSWORD"));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "ERROR");
    EXPECT_EQ((*msgs)[0]["code"], "WRONG_PASSWORD");
    EXPECT_EQ((*msgs)[0]["content"], "WRONG_PASSWORD");  // code used as content

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSendOk, WithoutFrom) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    EXPECT_TRUE(Protocol::send_ok(write_fd, "LOGOUT_OK"));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "LOGOUT_OK");
    EXPECT_FALSE((*msgs)[0].contains("from"));  // "from" field absent

    close(read_fd);
    close(write_fd);
}

TEST(ProtocolSend, JsonWithNestedStructure) {
    auto [read_fd, write_fd] = make_pipe();
    ClientSession session(read_fd);

    json msg = {
        {"type", "HISTORY_RESP"},
        {"to", "__room__"},
        {"data", json::array({
            {{"from", "alice"}, {"content", "msg1"}, {"timestamp", 1000}},
            {{"from", "bob"},   {"content", "msg2"}, {"timestamp", 2000}}
        })}
    };
    EXPECT_TRUE(Protocol::send_msg(write_fd, msg));

    auto msgs = Protocol::recv_msgs(session);
    ASSERT_TRUE(msgs.has_value());
    ASSERT_EQ(msgs->size(), 1u);
    EXPECT_EQ((*msgs)[0]["type"], "HISTORY_RESP");
    ASSERT_EQ((*msgs)[0]["data"].size(), 2u);
    EXPECT_EQ((*msgs)[0]["data"][0]["from"], "alice");
    EXPECT_EQ((*msgs)[0]["data"][1]["from"], "bob");

    close(read_fd);
    close(write_fd);
}

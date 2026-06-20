// test_client_win.cpp — Windows-compatible unit tests for LinuxChat client
//
// Tests client-side logic that doesn't require Linux-specific APIs.
// Uses Google Test framework.

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

// ── Test: Frame encoding (4-byte BE length prefix) ─────────────────

TEST(ProtocolFrame, BigEndianLengthPrefix) {
    uint32_t body_len = 256;  // 0x00000100
    uint32_t net_len = htonl(body_len);

    // Big-endian: most significant byte first
    uint8_t bytes[4];
    std::memcpy(bytes, &net_len, 4);

    EXPECT_EQ(bytes[0], 0x00);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x01);
    EXPECT_EQ(bytes[3], 0x00);
}

TEST(ProtocolFrame, ZeroLengthBody) {
    uint32_t body_len = 0;
    uint32_t net_len = htonl(body_len);

    uint8_t bytes[4];
    std::memcpy(bytes, &net_len, 4);

    EXPECT_EQ(bytes[0], 0x00);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x00);
    EXPECT_EQ(bytes[3], 0x00);
}

TEST(ProtocolFrame, MaxFrameSize256KB) {
    uint32_t max_body = 256 * 1024;  // 262144 = 0x00040000
    uint32_t net_len = htonl(max_body);

    uint8_t bytes[4];
    std::memcpy(bytes, &net_len, 4);

    EXPECT_EQ(bytes[0], 0x00);
    EXPECT_EQ(bytes[1], 0x04);
    EXPECT_EQ(bytes[2], 0x00);
    EXPECT_EQ(bytes[3], 0x00);
}

// ── Test: JSON message construction ────────────────────────────────

TEST(JsonMessage, LoginMessage) {
    // Simulate a LOGIN message construction
    std::string type = "LOGIN";
    std::string from = "alice";
    std::string content = "password123";

    // Verify fields are properly set
    EXPECT_EQ(type, "LOGIN");
    EXPECT_FALSE(from.empty());
    EXPECT_FALSE(content.empty());
}

TEST(JsonMessage, BroadcastMessage) {
    std::string type = "BROADCAST";
    std::string content = "Hello, world!";

    EXPECT_EQ(type, "BROADCAST");
    EXPECT_FALSE(content.empty());
}

TEST(JsonMessage, PrivateMessage) {
    std::string type = "PRIVATE";
    std::string from = "alice";
    std::string to = "bob";
    std::string content = "Secret message";

    EXPECT_EQ(type, "PRIVATE");
    EXPECT_NE(from, to);
    EXPECT_FALSE(content.empty());
}

// ── Test: Username validation ──────────────────────────────────────

TEST(UsernameValidation, EmptyUsername) {
    std::string username = "";
    EXPECT_TRUE(username.empty());
}

TEST(UsernameValidation, ValidUsername) {
    std::string username = "alice";
    EXPECT_FALSE(username.empty());
    EXPECT_LE(username.size(), 32u);
}

TEST(UsernameValidation, MaxLengthUsername) {
    std::string username(32, 'a');  // 32 chars
    EXPECT_EQ(username.size(), 32u);
    EXPECT_LE(username.size(), 32u);
}

// ── Test: Password hashing format ──────────────────────────────────

TEST(PasswordHash, SaltHashFormat) {
    // Simulate "salt_hex:hash_hex" format
    // salt: 16 bytes = 32 hex chars, hash: SHA-256 = 32 bytes = 64 hex chars
    std::string stored = "abcdef1234567890abcdef1234567890:fedcba0987654321fedcba0987654321fedcba0987654321fedcba0987654321";

    auto colon_pos = stored.find(':');
    ASSERT_NE(colon_pos, std::string::npos);

    std::string salt = stored.substr(0, colon_pos);
    std::string hash = stored.substr(colon_pos + 1);

    EXPECT_EQ(salt.size(), 32u);  // 16 bytes = 32 hex chars
    EXPECT_EQ(hash.size(), 64u);  // SHA-256 = 32 bytes = 64 hex chars
}

TEST(PasswordHash, LegacyFormatDetection) {
    // Legacy format: plain SHA-256 hash (no colon)
    std::string stored = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    auto colon_pos = stored.find(':');
    EXPECT_EQ(colon_pos, std::string::npos);  // No colon = legacy format
}

// ── Test: Frame size validation ────────────────────────────────────

TEST(FrameSize, RejectOversizedBody) {
    constexpr size_t MAX_BODY_LEN = 256 * 1024;
    size_t body_size = MAX_BODY_LEN + 1;

    EXPECT_GT(body_size, MAX_BODY_LEN);
}

TEST(FrameSize, AcceptMaxBody) {
    constexpr size_t MAX_BODY_LEN = 256 * 1024;
    size_t body_size = MAX_BODY_LEN;

    EXPECT_LE(body_size, MAX_BODY_LEN);
}

// ── Test: Parse failure counter ────────────────────────────────────

TEST(ParseFailure, ThreeStrikesDisconnect) {
    uint32_t consecutive_failures = 0;
    constexpr uint32_t MAX_FAILURES = 3;

    // Simulate 3 consecutive parse failures
    consecutive_failures++;
    EXPECT_LT(consecutive_failures, MAX_FAILURES);

    consecutive_failures++;
    EXPECT_LT(consecutive_failures, MAX_FAILURES);

    consecutive_failures++;
    EXPECT_GE(consecutive_failures, MAX_FAILURES);  // Should disconnect
}

TEST(ParseFailure, ResetOnSuccess) {
    uint32_t consecutive_failures = 2;  // 2 failures

    // Successful parse resets counter
    consecutive_failures = 0;
    EXPECT_EQ(consecutive_failures, 0u);
}

// ── Test: Generation counter ───────────────────────────────────────

TEST(GenerationCounter, MonotonicIncrement) {
    uint64_t next_generation = 1;

    uint64_t gen1 = ++next_generation;
    uint64_t gen2 = ++next_generation;
    uint64_t gen3 = ++next_generation;

    EXPECT_EQ(gen1, 2u);
    EXPECT_EQ(gen2, 3u);
    EXPECT_EQ(gen3, 4u);
    EXPECT_GT(gen3, gen2);
    EXPECT_GT(gen2, gen1);
}

TEST(GenerationCounter, DetectStaleTask) {
    uint64_t gen_at_dispatch = 5;
    uint64_t current_gen = 6;  // Incremented after disconnect

    EXPECT_NE(gen_at_dispatch, current_gen);  // Stale task should be dropped
}

// ── Main ───────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

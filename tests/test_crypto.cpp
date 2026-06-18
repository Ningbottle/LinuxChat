// test_crypto.cpp — Unit tests for SHA-256 hashing (MessageRouter::sha256_hex)
//
// Tests known vectors from NIST/FIPS 180-4 to verify correctness.
// Uses the static method directly — no Database or EpollServer needed.

#include <gtest/gtest.h>
#include "message_router.h"

// ── Known SHA-256 Vectors (NIST) ───────────────────────────────────

TEST(CryptoTest, SHA256_EmptyString) {
    EXPECT_EQ(MessageRouter::sha256_hex(""),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(CryptoTest, SHA256_HelloWorld) {
    EXPECT_EQ(MessageRouter::sha256_hex("hello"),
              "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(CryptoTest, SHA256_PasswordHash) {
    std::string hash = MessageRouter::sha256_hex("password123");
    EXPECT_EQ(hash.length(), 64u);  // SHA-256 always 64 hex chars
    EXPECT_NE(hash, MessageRouter::sha256_hex("password124"));  // Different input -> different hash
}

TEST(CryptoTest, SHA256_Deterministic) {
    std::string input = "test_input_data";
    EXPECT_EQ(MessageRouter::sha256_hex(input), MessageRouter::sha256_hex(input));
}

TEST(CryptoTest, SHA256_Unicode) {
    std::string hash = MessageRouter::sha256_hex("你好世界");
    EXPECT_EQ(hash.length(), 64u);
    EXPECT_NE(hash, MessageRouter::sha256_hex("hello"));
}

TEST(CryptoTest, SHA256_LongString) {
    std::string long_str(10000, 'a');
    std::string hash = MessageRouter::sha256_hex(long_str);
    EXPECT_EQ(hash.length(), 64u);
}

TEST(CryptoTest, SHA256_SingleChar) {
    EXPECT_EQ(MessageRouter::sha256_hex("a"),
              "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");
}

TEST(CryptoTest, SHA256_NumericString) {
    std::string hash1 = MessageRouter::sha256_hex("1234567890");
    std::string hash2 = MessageRouter::sha256_hex("0987654321");
    EXPECT_EQ(hash1.length(), 64u);
    EXPECT_NE(hash1, hash2);
}

TEST(CryptoTest, SHA256_PasswordSimilarInputs) {
    // Similar passwords should produce completely different hashes
    std::string h1 = MessageRouter::sha256_hex("password");
    std::string h2 = MessageRouter::sha256_hex("Password");
    std::string h3 = MessageRouter::sha256_hex("passworD");
    EXPECT_NE(h1, h2);
    EXPECT_NE(h1, h3);
    EXPECT_NE(h2, h3);
}

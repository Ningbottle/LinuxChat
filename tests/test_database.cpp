// test_database.cpp — Unit tests for Database layer (RED phase)
//
// Uses SQLite :memory: for fast, isolated tests.
// These tests define the expected Database API before implementation.

#include <gtest/gtest.h>
#include "database.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ── Fixture ────────────────────────────────────────────────────────

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
    }

    std::unique_ptr<Database> db;
};

// ── User Registration Tests ────────────────────────────────────────

TEST_F(DatabaseTest, RegisterUser_NewUser_ReturnsTrue) {
    EXPECT_TRUE(db->register_user("alice", "sha256_hash_of_password"));
}

TEST_F(DatabaseTest, RegisterUser_DuplicateUsername_ReturnsFalse) {
    ASSERT_TRUE(db->register_user("alice", "hash1"));
    EXPECT_FALSE(db->register_user("alice", "hash2"));
}

TEST_F(DatabaseTest, RegisterUser_DifferentUsernames_BothSucceed) {
    EXPECT_TRUE(db->register_user("alice", "hash1"));
    EXPECT_TRUE(db->register_user("bob", "hash2"));
}

// ── User Verification Tests ────────────────────────────────────────

TEST_F(DatabaseTest, VerifyUser_CorrectPassword_ReturnsTrue) {
    ASSERT_TRUE(db->register_user("alice", "correct_hash"));
    EXPECT_TRUE(db->verify_user("alice", "correct_hash"));
}

TEST_F(DatabaseTest, VerifyUser_WrongPassword_ReturnsFalse) {
    ASSERT_TRUE(db->register_user("alice", "correct_hash"));
    EXPECT_FALSE(db->verify_user("alice", "wrong_hash"));
}

TEST_F(DatabaseTest, VerifyUser_NonexistentUser_ReturnsFalse) {
    EXPECT_FALSE(db->verify_user("nobody", "any_hash"));
}

// ── Message Storage Tests ──────────────────────────────────────────

TEST_F(DatabaseTest, StoreMessage_Broadcast_CanRetrieve) {
    db->store_message("alice", "__room__", "hello everyone", 1700000000);

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[0]["content"], "hello everyone");
    EXPECT_EQ(history[0]["timestamp"], 1700000000);
}

TEST_F(DatabaseTest, StoreMessage_Private_CanRetrieve) {
    db->store_message("alice", "bob", "hi bob", 1700000001);

    auto history = db->get_history("bob");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["from"], "alice");
    EXPECT_EQ(history[0]["content"], "hi bob");
}

TEST_F(DatabaseTest, StoreMessage_MultipleOrderedByTimestamp) {
    db->store_message("alice", "__room__", "first", 1000);
    db->store_message("bob", "__room__", "second", 2000);
    db->store_message("charlie", "__room__", "third", 3000);

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 3u);
    // Should be ordered by timestamp ascending
    EXPECT_EQ(history[0]["timestamp"], 1000);
    EXPECT_EQ(history[1]["timestamp"], 2000);
    EXPECT_EQ(history[2]["timestamp"], 3000);
}

TEST_F(DatabaseTest, GetHistory_WithLimit_ReturnsLimited) {
    // Store 10 messages
    for (int i = 0; i < 10; ++i) {
        db->store_message("user" + std::to_string(i), "__room__",
                          "msg" + std::to_string(i), 1000 + i);
    }

    auto history = db->get_history("__room__", 5);
    EXPECT_EQ(history.size(), 5u);
}

TEST_F(DatabaseTest, GetHistory_DefaultLimit_Returns20) {
    // Store 25 messages
    for (int i = 0; i < 25; ++i) {
        db->store_message("user", "__room__",
                          "msg" + std::to_string(i), 1000 + i);
    }

    auto history = db->get_history("__room__");
    EXPECT_EQ(history.size(), 20u);
}

TEST_F(DatabaseTest, GetHistory_EmptyRoom_ReturnsEmpty) {
    auto history = db->get_history("__room__");
    EXPECT_TRUE(history.empty());
}

TEST_F(DatabaseTest, GetHistory_PrivateAndBroadcast_Separated) {
    db->store_message("alice", "__room__", "broadcast msg", 1000);
    db->store_message("alice", "bob", "private msg", 2000);

    auto room_history = db->get_history("__room__");
    auto bob_history = db->get_history("bob");

    ASSERT_EQ(room_history.size(), 1u);
    EXPECT_EQ(room_history[0]["content"], "broadcast msg");

    ASSERT_EQ(bob_history.size(), 1u);
    EXPECT_EQ(bob_history[0]["content"], "private msg");
}

// ── Edge Cases ─────────────────────────────────────────────────────

TEST_F(DatabaseTest, RegisterUser_EmptyUsername_ReturnsFalse) {
    EXPECT_FALSE(db->register_user("", "hash"));
}

TEST_F(DatabaseTest, StoreMessage_EmptyContent_StillStored) {
    db->store_message("alice", "__room__", "", 1000);
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["content"], "");
}

// ── Limit Clamping ─────────────────────────────────────────────────

TEST_F(DatabaseTest, GetHistory_LimitClampedTo200) {
    for (int i = 0; i < 250; ++i) {
        db->store_message("user", "__room__", "msg" + std::to_string(i), 1000 + i);
    }
    auto history = db->get_history("__room__", 250);
    EXPECT_EQ(history.size(), 200u);
}

TEST_F(DatabaseTest, GetHistory_LimitOne) {
    db->store_message("alice", "__room__", "first", 1000);
    db->store_message("bob", "__room__", "second", 2000);
    auto history = db->get_history("__room__", 1);
    EXPECT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["content"], "first");
}

TEST_F(DatabaseTest, GetHistory_LimitZero_ClampedToOne) {
    db->store_message("alice", "__room__", "msg", 1000);
    auto history = db->get_history("__room__", 0);
    EXPECT_EQ(history.size(), 1u);
}

TEST_F(DatabaseTest, GetHistory_NegativeLimit_ClampedToOne) {
    db->store_message("alice", "__room__", "msg", 1000);
    auto history = db->get_history("__room__", -5);
    EXPECT_EQ(history.size(), 1u);
}

// ── Large Content ──────────────────────────────────────────────────

TEST_F(DatabaseTest, StoreMessage_VeryLongContent) {
    std::string long_content(100000, 'x');
    db->store_message("alice", "__room__", long_content, 1000);
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["content"], long_content);
}

TEST_F(DatabaseTest, StoreMessage_UnicodeContent) {
    db->store_message("alice", "__room__", "你好世界🌍", 1000);
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["content"], "你好世界🌍");
}

TEST_F(DatabaseTest, StoreMessage_SpecialCharacters) {
    std::string special = "line1\nline2\ttab\"quotes\"'single'";
    db->store_message("alice", "__room__", special, 1000);
    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0]["content"], special);
}

// ── Multiple Users ─────────────────────────────────────────────────

TEST_F(DatabaseTest, RegisterUser_ManyUsers) {
    for (int i = 0; i < 50; ++i) {
        EXPECT_TRUE(db->register_user("user" + std::to_string(i), "hash"));
    }
    // All should verify
    for (int i = 0; i < 50; ++i) {
        EXPECT_TRUE(db->verify_user("user" + std::to_string(i), "hash"));
    }
}

// ── Timestamp Ordering ─────────────────────────────────────────────

TEST_F(DatabaseTest, GetHistory_MessagesOrderedByTimestamp) {
    db->store_message("alice", "__room__", "third", 3000);
    db->store_message("bob", "__room__", "first", 1000);
    db->store_message("charlie", "__room__", "second", 2000);

    auto history = db->get_history("__room__");
    ASSERT_EQ(history.size(), 3u);
    EXPECT_EQ(history[0]["content"], "first");
    EXPECT_EQ(history[1]["content"], "second");
    EXPECT_EQ(history[2]["content"], "third");
}

// ── Private Message Isolation ──────────────────────────────────────

TEST_F(DatabaseTest, GetHistory_PrivateMessages_Isolated) {
    db->store_message("alice", "bob", "msg for bob", 1000);
    db->store_message("alice", "charlie", "msg for charlie", 2000);

    auto bob_history = db->get_history("bob");
    auto charlie_history = db->get_history("charlie");

    ASSERT_EQ(bob_history.size(), 1u);
    EXPECT_EQ(bob_history[0]["content"], "msg for bob");

    ASSERT_EQ(charlie_history.size(), 1u);
    EXPECT_EQ(charlie_history[0]["content"], "msg for charlie");
}

// ── Password Hash Storage ──────────────────────────────────────────

TEST_F(DatabaseTest, VerifyUser_CaseSensitiveHash) {
    db->register_user("alice", "ABC123");
    EXPECT_TRUE(db->verify_user("alice", "ABC123"));
    EXPECT_FALSE(db->verify_user("alice", "abc123"));
}

TEST_F(DatabaseTest, VerifyUser_EmptyHash_NoMatch) {
    db->register_user("alice", "hash");
    EXPECT_FALSE(db->verify_user("alice", ""));
}

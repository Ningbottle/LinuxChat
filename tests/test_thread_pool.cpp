// test_thread_pool.cpp — Unit tests for ThreadPool
//
// Tests task execution, concurrency, and lifecycle.
// Uses std::promise/std::future for synchronization (enqueue returns void).

#include <gtest/gtest.h>
#include "thread_pool.h"
#include <atomic>
#include <future>
#include <vector>
#include <chrono>
#include <thread>

// ── Basic Functionality ────────────────────────────────────────────

TEST(ThreadPoolTest, BasicTaskExecution) {
    ThreadPool pool(4);
    std::promise<int> promise;
    auto future = promise.get_future();

    pool.enqueue([&promise] { promise.set_value(42); });

    EXPECT_EQ(future.get(), 42);
}

TEST(ThreadPoolTest, Size) {
    ThreadPool pool(8);
    EXPECT_EQ(pool.size(), 8u);
}

TEST(ThreadPoolTest, SingleThread) {
    ThreadPool pool(1);
    std::promise<int> promise;
    auto future = promise.get_future();

    pool.enqueue([&promise] { promise.set_value(99); });

    EXPECT_EQ(future.get(), 99);
}

// ── Multiple Tasks ─────────────────────────────────────────────────

TEST(ThreadPoolTest, MultipleTasks_AllExecuted) {
    ThreadPool pool(4);
    constexpr int NUM_TASKS = 100;
    std::atomic<int> counter{0};
    std::vector<std::promise<void>> promises(NUM_TASKS);
    std::vector<std::future<void>> futures;

    for (auto& p : promises) {
        futures.push_back(p.get_future());
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&counter, &promises, i] {
            counter++;
            promises[i].set_value();
        });
    }

    for (auto& f : futures) f.wait();
    EXPECT_EQ(counter.load(), NUM_TASKS);
}

TEST(ThreadPoolTest, MultipleTasks_WithReturnValues) {
    ThreadPool pool(4);
    constexpr int NUM_TASKS = 50;
    std::vector<std::promise<int>> promises(NUM_TASKS);
    std::vector<std::future<int>> futures;

    for (auto& p : promises) {
        futures.push_back(p.get_future());
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&promises, i] {
            promises[i].set_value(i * 2);
        });
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        EXPECT_EQ(futures[i].get(), i * 2);
    }
}

// ── Concurrent Access ──────────────────────────────────────────────

TEST(ThreadPoolTest, ConcurrentAccess_NoDataRace) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    constexpr int NUM_TASKS = 200;
    std::vector<std::promise<void>> promises(NUM_TASKS);
    std::vector<std::future<void>> futures;

    for (auto& p : promises) {
        futures.push_back(p.get_future());
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&counter, &promises, i] {
            // Simulate some work
            for (int j = 0; j < 100; ++j) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
            promises[i].set_value();
        });
    }

    for (auto& f : futures) f.wait();
    EXPECT_EQ(counter.load(), NUM_TASKS * 100);
}

TEST(ThreadPoolTest, ConcurrentAccess_OrderIndependent) {
    ThreadPool pool(4);
    std::vector<std::promise<int>> promises(50);
    std::vector<std::future<int>> futures;

    for (auto& p : promises) {
        futures.push_back(p.get_future());
    }

    for (int i = 0; i < 50; ++i) {
        pool.enqueue([&promises, i] {
            promises[i].set_value(i * i);
        });
    }

    // Results may arrive in any order, but each promise gets the right value
    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(futures[i].get(), i * i);
    }
}

// ── Lifecycle ──────────────────────────────────────────────────────

TEST(ThreadPoolTest, TasksQueuedBeforeDestructionAreExecuted) {
    std::atomic<int> counter{0};
    constexpr int NUM_TASKS = 10;

    {
        ThreadPool pool(4);
        for (int i = 0; i < NUM_TASKS; ++i) {
            pool.enqueue([&counter] { counter++; });
        }
        // Destructor runs here: sets stop_, notifies workers, joins.
        // Workers drain remaining tasks before exiting.
    }

    EXPECT_EQ(counter.load(), NUM_TASKS);
}

TEST(ThreadPoolTest, DestructorJoinsAllWorkers) {
    // Verify that the destructor doesn't crash or hang
    std::atomic<int> counter{0};

    for (int round = 0; round < 5; ++round) {
        {
            ThreadPool pool(4);
            for (int i = 0; i < 10; ++i) {
                pool.enqueue([&counter] { counter++; });
            }
        }
    }

    EXPECT_EQ(counter.load(), 50);
}

// ── Edge Cases ─────────────────────────────────────────────────────

TEST(ThreadPoolTest, ZeroThreads_ThrowsInvalidArgument) {
    EXPECT_THROW(ThreadPool(0), std::invalid_argument);
}

TEST(ThreadPoolTest, ManyTasks_FewThreads) {
    ThreadPool pool(2);
    constexpr int NUM_TASKS = 100;
    std::atomic<int> counter{0};
    std::vector<std::promise<void>> promises(NUM_TASKS);
    std::vector<std::future<void>> futures;

    for (auto& p : promises) {
        futures.push_back(p.get_future());
    }

    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.enqueue([&counter, &promises, i] {
            counter++;
            promises[i].set_value();
        });
    }

    for (auto& f : futures) f.wait();
    EXPECT_EQ(counter.load(), NUM_TASKS);
}

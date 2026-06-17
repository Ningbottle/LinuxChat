#pragma once
// thread_pool.h — Fixed-size thread pool with task queue
// Used by EpollServer to offload business logic from the event loop

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    /// Constructs a thread pool with `num_threads` worker threads.
    explicit ThreadPool(size_t num_threads);

    /// Destructor: signals stop and joins all workers.
    ~ThreadPool();

    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// Enqueue a task for execution by any worker thread.
    /// Throws std::runtime_error if called after stop.
    void enqueue(std::function<void()> task);

    /// Returns the number of worker threads.
    size_t size() const { return workers_.size(); }

private:
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex                        mutex_;
    std::condition_variable           cv_;
    bool                              stop_ = false;
};

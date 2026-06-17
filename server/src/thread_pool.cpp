// thread_pool.cpp — ThreadPool implementation

#include "thread_pool.h"
#include <stdexcept>

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads == 0) {
        throw std::invalid_argument("ThreadPool: num_threads must be > 0");
    }

    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] {
            // Worker thread loop
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    // Wait until there's a task or we're stopping
                    cv_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                    });

                    if (stop_ && tasks_.empty()) {
                        return; // Exit worker thread
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                // Execute task outside the lock
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all(); // Wake all workers so they can exit
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool: enqueue called after stop");
        }
        tasks_.emplace(std::move(task));
    }
    cv_.notify_one(); // Wake one worker
}

// This is a queue that can be accessed from multiple threads safely.
//
// It's not well optimized, and requires obtaining a lock every time you check for a new value.

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <class T>
class SynchronizedQueue {
    std::condition_variable cv_;
    std::mutex lock_;
    std::queue<T> q_;
public:

    void push(T && t) {
        std::unique_lock l(lock_);
        q_.push(std::move(t));
        cv_.notify_one();
    }

    std::optional<T> get() {
        std::unique_lock l(lock_);
        if(q_.empty()) {
            return std::nullopt;
        }

        T t = std::move(q_.front());
        q_.pop();
        return t;
    }

    T get_blocking() {
        std::unique_lock l(lock_);
        while(q_.empty()) {
            cv_.wait(l);
        }

        T t = std::move(q_.front());
        q_.pop();
        return std::move(t);
    }
};
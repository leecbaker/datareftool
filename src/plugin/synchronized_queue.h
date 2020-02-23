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
    std::atomic_uint64_t input_count{0}, output_count{0};
public:

    void push(T && t) {
        std::unique_lock l(lock_);
        q_.push(std::move(t));
        input_count++;
        cv_.notify_one();
    }

    std::optional<T> get() {
        if(input_count == output_count) {
            return std::nullopt;
        }
        {
            std::unique_lock l(lock_);
            if(q_.empty()) {
                return std::nullopt;
            }

            output_count++;
            T t = std::move(q_.front());
            q_.pop();
            return t;
        }
    }

    T get_blocking() {
        std::unique_lock l(lock_);
        while(q_.empty()) {
            cv_.wait(l);
        }

        output_count++;
        T t = std::move(q_.front());
        q_.pop();
        return std::move(t);
    }
};
#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <utility>

template <typename T>
class ThreadSafeQueue {
    public:
        explicit ThreadSafeQueue(std::size_t max_size): max_size_{max_size} {}

        void push(T object) {
            const std::lock_guard<std::mutex> lock(mutex_);

            if (max_size_ == 0) {
                return;
            }

            queue_.push(std::move(object));
            if (queue_.size() > max_size_) {
                queue_.pop(); // TODO add logging (warning)
            }
        }

        T back() const {
            const std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                throw std::out_of_range("ThreadSafeQueue is empty");
            }
            return queue_.back();
        }

        void pop() {
            const std::lock_guard<std::mutex> lock(mutex_);
            if (!queue_.empty()) {
                queue_.pop();
            }
        }

        bool empty() { return queue_.empty(); }

        std::queue<T> snapshot() const {
            const std::lock_guard<std::mutex> lock(mutex_);
            return queue_;
        }

    private:
        mutable std::mutex mutex_;
        std::size_t max_size_;
        std::queue<T> queue_;
};

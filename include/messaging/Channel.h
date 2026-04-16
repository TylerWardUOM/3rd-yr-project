// messaging/Channel.h
#pragma once

#include <queue>
#include <mutex>
#include <vector>
#include <utility>

#include "ChannelBase.h"

namespace msg {

template<typename T>
class Channel final : public ChannelBase {
public:
    ChannelKind kind() const override {
        return ChannelKind::Queue;
    }
    Channel() = default;

    // Publish a message (copy)
    void publish(const T& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
    }

    // Publish a message (move)
    void publish(T&& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(msg));
    }

    // Try to consume a single message (non-blocking)
    bool tryConsume(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
            return false;

        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Drain all messages into a vector (common for physics step)
    void drain(std::vector<T>& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            out.push_back(std::move(queue_.front()));
            queue_.pop();
        }
    }

    // For diagnostics / debugging only
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
};

} // namespace msg

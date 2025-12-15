// messaging/MessageBus.h
#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>

#include "Channel.h"

namespace msg {

class MessageBus {
public:
    MessageBus() = default;
    ~MessageBus() = default;

    // Create or retrieve a channel by name and type
    template<typename T>
    Channel<T>& channel(const std::string& name) {
        auto it = channels_.find(name);
        if (it != channels_.end()) {
            auto* typed = dynamic_cast<Channel<T>*>(it->second.get());
            if (!typed)
                throw std::runtime_error("Channel type mismatch: " + name);
            return *typed;
        }

        auto ch = std::make_unique<Channel<T>>();
        Channel<T>* raw = ch.get();
        channels_[name] = std::move(ch);
        return *raw;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ChannelBase>> channels_;
};

} // namespace msg

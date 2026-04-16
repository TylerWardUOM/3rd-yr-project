#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>

#include "Channel.h"
#include "SnapshotChannel.h"
#include "ChannelBase.h"

namespace msg {

class MessageBus {
public:
    MessageBus() = default;
    ~MessageBus() = default;

    // -----------------------------
    // Queue channel (commands)
    // -----------------------------
    template<typename T>
    Channel<T>& channel(const std::string& name) {
        return getOrCreate<Channel<T>>(name, ChannelKind::Queue);
    }

    // -----------------------------
    // Snapshot channel (SPMC)
    // -----------------------------
    template<typename T>
    SnapshotChannel<T>& snapshot(const std::string& name) {
        return getOrCreate<SnapshotChannel<T>>(name, ChannelKind::Snapshot);
    }

private:
    template<typename ChannelT>
    ChannelT& getOrCreate(const std::string& name, ChannelKind expected) {
        auto it = channels_.find(name);
        if (it != channels_.end()) {
            if (it->second->kind() != expected)
                throw std::runtime_error("Channel kind mismatch: " + name);

            auto* typed = dynamic_cast<ChannelT*>(it->second.get());
            if (!typed)
                throw std::runtime_error("Channel type mismatch: " + name);

            return *typed;
        }

        auto ch = std::make_unique<ChannelT>();
        ChannelT* raw = ch.get();
        channels_[name] = std::move(ch);
        return *raw;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ChannelBase>> channels_;
};

} // namespace msg

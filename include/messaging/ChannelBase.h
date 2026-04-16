// messaging/ChannelBase.h
#pragma once

namespace msg {

enum class ChannelKind {
    Queue,
    Snapshot
};

class ChannelBase {
public:
    virtual ~ChannelBase() = default;
    virtual ChannelKind kind() const = 0;
};

} // namespace msg

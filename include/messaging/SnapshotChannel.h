#pragma once

namespace msg {

template<typename T>
class SnapshotChannel final : public ChannelBase {
public:
    ChannelKind kind() const override {
        return ChannelKind::Snapshot;
    }

    void publish(const T& msg) {
        uint32_t writeIdx = 1 - readIdx_.load(std::memory_order_relaxed);
        buffers_[writeIdx] = msg;
        readIdx_.store(writeIdx, std::memory_order_release);
        version_.fetch_add(1, std::memory_order_relaxed);
    }

    bool tryRead(T& out, uint64_t& lastVersion) const {
        uint64_t v = version_.load(std::memory_order_acquire);
        if (v == lastVersion)
            return false;

        uint32_t idx = readIdx_.load(std::memory_order_acquire);
        out = buffers_[idx];
        lastVersion = v;
        return true;
    }

private:
    T buffers_[2];
    std::atomic<uint32_t> readIdx_{0};
    std::atomic<uint64_t> version_{0};
};

}
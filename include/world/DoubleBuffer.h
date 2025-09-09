#pragma once
#include <atomic>

template <typename T>
struct DoubleBuffer {
    T A{}, B{};
    std::atomic<uint8_t> idx{0};
    void write(const T& v){ auto i=idx.load(std::memory_order_relaxed); (i?A:B)=v; idx.store(1u-i,std::memory_order_release); }
    T    read()  const   { auto i=idx.load(std::memory_order_acquire); return (i?B:A); }
};

#pragma once
#include <cstdint>

struct DeviceTimingLogMsg {
    uint64_t t_rx_parse_ns = 0;
    uint64_t t_tool_publish_ns = 0;
    uint64_t t_wrench_consume_ns = 0;
    uint64_t t_tx_start_ns = 0;
    uint64_t t_tx_done_ns = 0;

    uint32_t rx_seq = 0;

    float q1 = 0.0f;
    float q2 = 0.0f;

    float fx = 0.0f;
    float fy = 0.0f;

    float tau1 = 0.0f;
    float tau2 = 0.0f;
};
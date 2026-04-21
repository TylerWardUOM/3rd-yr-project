#pragma once

struct DeviceTimingLogMsg {
    // Host-side timestamps for a matched closed-loop control cycle
    uint64_t t_rx_parse_ns = 0;       // state packet used for this cycle was parsed
    uint64_t t_tool_publish_ns = 0;   // tool state published to host pipeline
    uint64_t t_wrench_consume_ns = 0; // newest haptic wrench consumed
    uint64_t t_tx_start_ns = 0;       // torque command transmission started
    uint64_t t_tx_done_ns = 0;        // torque command transmission finished

    // Matched state/command identity
    uint32_t rx_state_seq = 0;
    uint32_t state_mcu_us = 0;
    uint32_t tx_cmd_seq = 0;
    uint32_t ref_state_seq = 0;

    // State and command payloads used in this cycle
    float q1 = 0.0f;
    float q2 = 0.0f;
    float fx = 0.0f;
    float fy = 0.0f;
    float tau1 = 0.0f;
    float tau2 = 0.0f;
};

struct DeviceStateLogMsg {
    // Raw parsed incoming state-packet log
    uint64_t t_chunk_read_ns = 0; // most recent non-empty serial chunk read time
    uint64_t t_rx_parse_ns = 0;   // time this packet was parsed from incomingBuffer_
    uint32_t rx_state_seq = 0;
    uint32_t state_mcu_us = 0;
    float q1 = 0.0f;
    float q2 = 0.0f;
};
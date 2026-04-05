#pragma once
#include <cstdint>

struct DeviceTimingLogMsg {
    // Host-side timestamps
    uint64_t t_rx_parse_ns = 0;
    uint64_t t_tool_publish_ns = 0;
    uint64_t t_wrench_consume_ns = 0;
    uint64_t t_tx_start_ns = 0;
    uint64_t t_tx_done_ns = 0;

    // Packet identity / matching
    uint32_t rx_state_seq = 0;     // state packet parsed this cycle
    uint32_t state_mcu_us = 0;     // MCU timestamp embedded in parsed state packet
    uint32_t tx_cmd_seq = 0;       // command packet sequence sent by host
    uint32_t ref_state_seq = 0;    // state sequence the command was based on

    // Payload data
    float q1 = 0.0f;
    float q2 = 0.0f;

    float fx = 0.0f;
    float fy = 0.0f;

    float tau1 = 0.0f;
    float tau2 = 0.0f;
};

struct DeviceStateLogMsg {
    uint64_t t_rx_parse_ns = 0;
    uint32_t rx_state_seq = 0;
    uint32_t state_mcu_us = 0;
    float q1 = 0.0f;
    float q2 = 0.0f;
};
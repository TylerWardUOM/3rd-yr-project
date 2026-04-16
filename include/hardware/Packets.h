#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct DeviceStatePacket {
    uint8_t header[2];
    uint32_t state_seq;
    uint32_t t_mcu_us;
    float joint_angle[2];
    uint16_t checksum;
};

struct TorqueCommandPacket {
    uint8_t header[2];
    uint32_t cmd_seq;
    uint32_t ref_state_seq;
    float joint_torque[2];
    uint16_t checksum;
};

#pragma pack(pop)

#pragma pack(push,1)
struct RateTestPacket {
    uint16_t header;      // e.g. 0xAA55
    uint32_t seq;
    uint32_t t_mcu_us;    // optional
    float q1;
    float q2;
    uint16_t checksum;
};
#pragma pack(pop)

#pragma pack(push,1)
struct PingPacket {
    uint16_t header;      
    uint32_t seq;
    uint64_t t_host_send_ns;
    uint16_t checksum;
};
#pragma pack(pop)
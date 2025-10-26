#pragma once
#include <cstdint>

// For alignment consistency (packed structures)
#pragma pack(push, 1)

// MCU → Host
struct DeviceStatePacket {
    uint16_t header = 0xAA55; // packet header
    float joint_angle[2];  // radians
    uint16_t checksum;
};

// Host → MCU
struct TorqueCommandPacket {
    uint16_t header = 0x55AA; // packet header
    float joint_torque[2]; // N·m
    uint16_t checksum;
};

#pragma pack(pop)

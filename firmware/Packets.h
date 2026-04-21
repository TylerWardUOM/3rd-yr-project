#pragma pack(push, 1)
typedef struct {
    uint8_t header[2];
    uint32_t cmd_seq;
    uint32_t ref_state_seq;
    float joint_torque[2];
    uint16_t checksum;
} TorqueCommandPacket;

typedef struct {
    uint8_t header[2];
    uint32_t state_seq;
    uint32_t t_mcu_us;    // microsecond timestamp from MCU 
    float joint_angle[2];
    uint16_t checksum;
} DeviceStatePacket;
#pragma pack(pop)
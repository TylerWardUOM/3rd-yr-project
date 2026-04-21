#ifndef DEVICECOMMS_H
#define DEVICECOMMS_H

#include <Arduino.h>
#include <stdint.h>
#include "Packets.h"

#define UART_RX_SIZE 512

struct DeviceComms {
    HardwareSerial* serial;

    uint8_t rxBuf[UART_RX_SIZE];
    uint32_t rxHead;
    uint32_t rxTail;

    float commanded_torque[2];
    float angle[2];


    uint32_t last_command_us;

    // Outgoing state metadata
    uint32_t state_seq;
    uint32_t t_mcu_us;

    // Last received command metadata
    uint32_t last_cmd_seq;
    uint32_t last_ref_state_seq;
};

void DeviceComms_Init(DeviceComms* dc, HardwareSerial& serialPort);
void DeviceComms_SetState(DeviceComms *dc,
                          uint32_t state_seq,
                          uint32_t t_mcu_us,
                          float a0,
                          float a1);
void DeviceComms_Receive(DeviceComms* dc);
void DeviceComms_SendState(DeviceComms* dc);



#endif
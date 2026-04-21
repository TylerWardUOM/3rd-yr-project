#include "DeviceComms.h"
#include <string.h>

#define HEADER0 0xAA
#define HEADER1 0x55

static uint16_t checksumBytes(const uint8_t* data, uint32_t len)
{
    uint16_t s = 0;
    for (uint32_t i = 0; i < len; i++) {
        s += data[i];
    }
    return s;
}

void DeviceComms_Init(DeviceComms* dc, HardwareSerial& serialPort)
{
    dc->serial = &serialPort;
    dc->rxHead = 0;
    dc->rxTail = 0;

    dc->commanded_torque[0] = 0.0f;
    dc->commanded_torque[1] = 0.0f;

    dc->angle[0] = 0.0f;
    dc->angle[1] = 0.0f;

    dc->last_command_us = 0;

    dc->state_seq = 0;
    dc->t_mcu_us = 0;

    dc->last_cmd_seq = 0;
    dc->last_ref_state_seq = 0;
}

void DeviceComms_SetState(DeviceComms* dc,
                          uint32_t state_seq,
                          uint32_t t_mcu_us,
                          float a0,
                          float a1)
{
    dc->angle[0] = a0;
    dc->angle[1] = a1;
    dc->t_mcu_us = t_mcu_us;
    dc->state_seq = state_seq;
}

static uint32_t bytesAvailable(const DeviceComms* dc)
{
    return (dc->rxHead + UART_RX_SIZE - dc->rxTail) % UART_RX_SIZE;
}

static void clearRxBuffer(DeviceComms* dc)
{
    dc->rxHead = 0;
    dc->rxTail = 0;
}

static void pushByte(DeviceComms* dc, uint8_t b)
{
    uint32_t nextHead = (dc->rxHead + 1) % UART_RX_SIZE;

    // If full, drop oldest byte
    if (nextHead == dc->rxTail) {
        dc->rxTail = (dc->rxTail + 1) % UART_RX_SIZE;
    }

    dc->rxBuf[dc->rxHead] = b;
    dc->rxHead = nextHead;
}

static bool peekPacket(const DeviceComms* dc, TorqueCommandPacket* pkt)
{
    const uint32_t pktSize = sizeof(TorqueCommandPacket);

    if (bytesAvailable(dc) < pktSize) {
        return false;
    }

    for (uint32_t i = 0; i < pktSize; i++) {
        uint32_t idx = (dc->rxTail + i) % UART_RX_SIZE;
        ((uint8_t*)pkt)[i] = dc->rxBuf[idx];
    }

    return true;
}

static void dropBytes(DeviceComms* dc, uint32_t n)
{
    dc->rxTail = (dc->rxTail + n) % UART_RX_SIZE;
}

static bool tryParseLatestCommand(DeviceComms* dc)
{
    const uint32_t pktSize = sizeof(TorqueCommandPacket);
    bool gotAnyValidPacket = false;
    TorqueCommandPacket pkt{};

    // If massively behind, recover by clearing buffer
    if (bytesAvailable(dc) > UART_RX_SIZE / 2) {
        clearRxBuffer(dc);
        return false;
    }

    while (bytesAvailable(dc) >= pktSize) {
        if (!peekPacket(dc, &pkt)) {
            break;
        }

        // Header match?
        if (pkt.header[0] == HEADER0 && pkt.header[1] == HEADER1) {
            uint16_t chk = checksumBytes((uint8_t*)&pkt, pktSize - 2);

            if (chk == pkt.checksum) {
                // Keep newest valid command
                dc->commanded_torque[0] = pkt.joint_torque[0];
                dc->commanded_torque[1] = pkt.joint_torque[1];

                dc->last_cmd_seq = pkt.cmd_seq;
                dc->last_ref_state_seq = pkt.ref_state_seq;
                dc->last_command_us = micros();

                dropBytes(dc, pktSize);
                gotAnyValidPacket = true;
                continue;
            }
        }

        // Bad alignment or bad checksum: shift one byte and rescan
        dropBytes(dc, 1);
    }

    return gotAnyValidPacket;
}

void DeviceComms_Receive(DeviceComms* dc)
{
    while (dc->serial->available() > 0) {
        uint8_t b = (uint8_t)dc->serial->read();
        pushByte(dc, b);
    }

    // Parse everything buffered, keeping only latest valid command
    tryParseLatestCommand(dc);
}

void DeviceComms_SendState(DeviceComms* dc)
{
    DeviceStatePacket pkt{};
    pkt.header[0] = HEADER0;
    pkt.header[1] = HEADER1;

    pkt.state_seq = dc->state_seq;
    pkt.t_mcu_us = dc->t_mcu_us;

    pkt.joint_angle[0] = dc->angle[0];
    pkt.joint_angle[1] = dc->angle[1];

    pkt.checksum = checksumBytes((uint8_t*)&pkt, sizeof(DeviceStatePacket) - 2);

    dc->serial->write((uint8_t*)&pkt, sizeof(DeviceStatePacket));
}
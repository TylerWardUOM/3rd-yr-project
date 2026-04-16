# Communication Tests

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#Executable and Build Integration]]
- [[#Test Modes]]
- [[#Parser and Packet Validation]]
- [[#Rate Test (rate)]]
- [[#RTT Test (rtt)]]
- [[#CSV Outputs]]

---

## Overview

Serial communication tests are implemented in `src/main_serial_tests.cpp` as a standalone CLI tool.

The tool validates packet integrity and communication timing between host and firmware over the same serial link used by the device interface.

---

## Executable and Build Integration

`CMakeLists.txt` defines a dedicated executable:

- target: `serial_tests`
- sources: `src/main_serial_tests.cpp`, `src/hardware/SerialLink.cpp`

This keeps communication test logic separate from the main application executable (`app`).

---

## Test Modes

The executable supports two modes:

- `rate` — packet rate / ordering / drop analysis from streamed `RateTestPacket`
- `rtt` — ping-echo round-trip latency analysis using `PingPacket`

CLI usage (from `printUsage()`):

- `serial_tests rate <COMx> <baud> <duration_sec> <csv_file>`
- `serial_tests rtt  <COMx> <baud> <count> <interval_ms> <csv_file>`

---

## Parser and Packet Validation

Both modes use buffered parsing with:

- byte-stream header search (`0xAA55` for rate packets, `0xCC33` for ping packets)
- checksum validation before accepting a packet
- resynchronisation by dropping bytes when malformed data is detected

This makes tests robust against serial fragmentation and corrupted bytes.

---

## Rate Test (rate)

`runRateTest()`:

1. Connects to serial (`SerialLink::connect`)
2. Reads incoming bytes continuously
3. Parses `RateTestPacket` frames
4. Tracks packet quality metrics:
   - received packet count
   - dropped packets (`seq` gaps)
   - out-of-order packets
   - inter-arrival timing (`dt`)
5. Writes per-packet rows to CSV and prints a summary (rate, jitter, min/max dt)

---

## RTT Test (rtt)

`runRttTest()`:

1. Sends sequenced ping packets with host timestamp
2. Waits for matching echo replies (200 ms timeout per ping)
3. Computes RTT from host send to host receive
4. Records success/timeout and RTT metrics
5. Prints summary statistics (mean, median, p95, p99, min, max)

---

## CSV Outputs

- Rate mode CSV columns:
  - `arrival_ns,seq,t_mcu_us,q1,q2,dt_arrival_ms,dropped_since_last,out_of_order`
- RTT mode CSV columns:
  - `seq,send_ns,recv_ns,rtt_ms,success`

These CSV outputs are designed for offline plotting and communication quality analysis.

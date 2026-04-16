# Logging

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#Log Message Types]]
- [[#Runtime Logging Pipeline]]
- [[#Shutdown CSV Export]]
- [[#Output Files]]

---

## Overview

Logging is implemented as a non-blocking message pipeline from the device thread into a dedicated log thread.

The goal is to capture timing-critical host/device communication data without doing file I/O in the 1 kHz device loop.

---

## Log Message Types

Log payloads are defined in `include/data/LogMessages.h`:

- `DeviceTimingLogMsg` — full per-cycle timing and signal data:
  - host timestamps (`t_rx_parse_ns`, `t_tool_publish_ns`, `t_wrench_consume_ns`, `t_tx_start_ns`, `t_tx_done_ns`)
  - packet identity fields (`rx_state_seq`, `state_mcu_us`, `tx_cmd_seq`, `ref_state_seq`)
  - signal values (`q1`, `q2`, `fx`, `fy`, `tau1`, `tau2`)
- `DeviceStateLogMsg` — per-parsed-state sample:
  - `t_rx_parse_ns`, `rx_state_seq`, `state_mcu_us`, `q1`, `q2`

---

## Runtime Logging Pipeline

`main.cpp` creates two logging channels on the `MessageBus`:

- `logging.device_timing` (`Channel<DeviceTimingLogMsg>`)
- `logging.device_state` (`Channel<DeviceStateLogMsg>`)

`DeviceAdapter::update()` publishes logs:

1. Parsed incoming state packets produce `DeviceStateLogMsg` entries.
2. A matched control cycle (valid state + consumed wrench + successful send + matching sequence reference) produces `DeviceTimingLogMsg`.

A dedicated log thread in `main.cpp` continuously drains both channels and appends entries to in-memory vectors:

- `timingLogs`
- `stateLogs`

This keeps the device thread free from disk latency while preserving all captured samples.

---

## Shutdown CSV Export

After threads stop and the log thread is joined, `main.cpp` writes CSV files:

- `device_timing.csv`
- `device_state_log.csv`

The CSV header is written first, then all buffered messages are dumped row-by-row in capture order.

---

## Output Files

- `device_timing.csv` — end-to-end cycle timing + wrench/torque signals for matched rx/tx cycles
- `device_state_log.csv` — raw parsed state stream from firmware packets

These files are used for offline analysis in `Test Data/End_to_End.m`.

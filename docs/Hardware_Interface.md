# Hardware Interface

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#SerialLink]]
- [[#Packets]]
- [[#DeviceAdapter]]
- [[#Forward Kinematics (anglesToPose)]]
- [[#Jacobian and Torque Computation]]
- [[#Timing and Logging]]

---

## Overview

The hardware interface layer connects the physical haptic device (a 2-DOF planar robot arm with encoders and motors) to the simulation.

Two classes handle this:
- `SerialLink` — raw Windows serial (COM) port I/O
- `DeviceAdapter` — packet parsing, forward kinematics, Jacobian torque computation, and message channel bridging

The device communicates at **460800 baud** on a COM port.  
It sends encoder angle packets at ~1 kHz and receives torque command packets at the same rate.

`DeviceAdapter::update()` is called from a dedicated device thread at 1 kHz (see [[Implementation_Index#Architecture Overview]]).

---

## SerialLink

`SerialLink` (`Rewrite/include/hardware/SerialLink.h`) is a thin wrapper around a Windows `HANDLE`-based serial connection.

API:
- `connect(portName, baud)` — open COM port, configure baud rate
- `disconnect()` — close handle
- `sendBytes` / `sendRaw` — write to serial
- `readBytes` / `readAvailable` — read from serial (non-blocking, reads up to `maxBytes`)
- `isConnected()` — state check

The `HANDLE` is stored as `void*` to avoid pulling Windows headers into other translation units.

---

## Packets

`Packets.h` (`Rewrite/include/hardware/Packets.h`) defines the wire format structs, all `#pragma pack(push, 1)` (no padding):

### DeviceStatePacket (firmware → host)

| Field | Type | Description |
|---|---|---|
| `header[2]` | uint8 | `0xAA 0x55` |
| `state_seq` | uint32 | monotonic sequence number |
| `t_mcu_us` | uint32 | MCU timestamp in microseconds |
| `joint_angle[2]` | float | q1, q2 in radians |
| `checksum` | uint16 | byte sum of all preceding bytes |

### TorqueCommandPacket (host → firmware)

| Field | Type | Description |
|---|---|---|
| `header[2]` | uint8 | `0xAA 0x55` |
| `cmd_seq` | uint32 | command sequence |
| `ref_state_seq` | uint32 | state packet this references |
| `joint_torque[2]` | float | τ1, τ2 in N·m |
| `checksum` | uint16 | byte sum of all preceding bytes |

Checksum is a simple unsigned byte sum of all bytes except the last two.

---

## DeviceAdapter

`DeviceAdapter` (`Rewrite/include/hardware/DeviceAdapter.h`) bridges hardware I/O to the messaging layer.

Constructor inputs:
- `deviceIn` channel — publishes `ToolStateMsg` (tool pose from FK)
- `deviceCmdOut` channel — consumes `HapticWrenchCmd` (force to convert to torques)
- `timingLogOut`, `stateLogOut` — diagnostic log channels

`update(timeNow)` runs once per device-thread tick:

1. **Read serial** — `link_.readAvailable(chunk)` → append to `incomingBuffer_`
2. **Parse packets** — `tryParseOnePacket()` in a loop:
   - scan buffer for `0xAA 0x55` header
   - verify checksum
   - consume packet from buffer if valid
   - log to `stateLogOut_`
   - keep only the **newest** valid packet per tick
3. **Forward kinematics** — if new state received: `anglesToPose(latestAngles_)` → `ToolStateMsg` → publish to `deviceIn_`
4. **Consume wrench command** — drain `deviceCmdOut_` channel, keep newest
5. **Compute torques** — `computeJacobiansAndTorques(angles, force, pkt_out, log)`
6. **Send torque packet** — `link_.sendRaw(pkt_out)`
7. **Timing log** — if a matched rx/tx cycle completed, publish `DeviceTimingLogMsg`

---

## Forward Kinematics (anglesToPose)

`anglesToPose(float q[2])` converts joint angles to end-effector pose in world space.

2-DOF planar arm, link lengths L1 = L2 = 0.15 m:

```
x = L1*cos(q1) + L2*cos(q1+q2)
y = L1*sin(q1) + L2*sin(q1+q2)
```

Orientation quaternion = rotation by `q1+q2` about Z axis.

The resulting `Pose` is published as `ToolStateMsg.toolPose_ws` → consumed by [[Haptic Engine]].

---

## Jacobian and Torque Computation

`computeJacobiansAndTorques(q, wrenchCmd, pkt_out, log)` converts a Cartesian force to joint torques.

The 2-DOF planar Jacobian relates joint velocities to end-effector velocities:

```
J = [ -L1*sin(q1) - L2*sin(q1+q2),   -L2*sin(q1+q2) ]
    [  L1*cos(q1) + L2*cos(q1+q2),    L2*cos(q1+q2)  ]
```

Joint torques from Cartesian force: `τ = Jᵀ · F`

```
τ1 = J11*Fx + J21*Fy
τ2 = J12*Fx + J22*Fy
```

The force `[Fx, Fy]` comes from the latest `HapticWrenchCmd.force_ws` (world-space XY plane).

Output torques are negated/adjusted for motor sign convention and packed into `TorqueCommandPacket`.

---

## Timing and Logging

The device thread runs tight at 1 kHz, so timing matters.

`DeviceTimingLogMsg` captures per-cycle timestamps (in nanoseconds from `steady_clock`):
- `t_rx_parse_ns` — when the state packet was parsed
- `t_tool_publish_ns` — when `ToolStateMsg` was published
- `t_wrench_consume_ns` — when the haptic wrench was consumed
- `t_tx_start_ns` / `t_tx_done_ns` — serial write timestamps
- `state_mcu_us` — MCU-side timestamp from the device (for latency measurement)
- `q1`, `q2`, `fx`, `fy`, `tau1`, `tau2` — signal values for that cycle

A matched cycle (valid rx + valid wrench + successful send + sequence numbers match) triggers a log entry.

On shutdown, `main.cpp` writes all timing logs to `device_timing.csv` for analysis.  
`DeviceStateLogMsg` (every parsed packet) is written to `device_state_log.csv`.

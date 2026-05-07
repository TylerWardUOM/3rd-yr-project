# Messaging

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#Channel (Queue)]]
- [[#SnapshotChannel (Latest-Value)]]
- [[#MessageBus]]
- [[#Named Channels in Use]]
- [[Thread_Message_Bus_Diagram]]

---

## Overview

The messaging layer provides thread-safe communication between the simulation, haptic, device, and render subsystems.

All channels are created at startup via `MessageBus` and passed by reference into the subsystems that need them.  
No subsystem holds a raw pointer to another — they only hold channel references.

Two primitives:
- **`Channel<T>`** — a thread-safe FIFO queue for command-style messages
- **`SnapshotChannel<T>`** — a double-buffered latest-value store for high-frequency state

---

## Channel (Queue)

`Channel<T>` (`include/messaging/Channel.h`) is a mutex-protected `std::queue<T>`.

API:
- `publish(msg)` — enqueue a message (copy or move)
- `tryConsume(out)` — dequeue one message if available (non-blocking, returns bool)
- `drain(vec)` — move all queued messages into a vector (used by physics for batch processing)
- `size()` — current queue depth (diagnostics only)

Use cases:
- world commands from UI → `WorldManager`
- haptic wrench commands → `PhysicsEngine` and `DeviceAdapter`
- tool state from device → `HapticEngine`
- haptic snapshots from `HapticEngine` → renderer
- timing/state logs → log thread

The mutex ensures safe concurrent publish + consume from different threads.  
Messages accumulate between consumer ticks — the consumer drains everything each tick (e.g., physics drains all wrenches, applies them, then simulates).

---

## SnapshotChannel (Latest-Value)

`SnapshotChannel<T>` (`include/messaging/SnapshotChannel.h`) is a **lock-free double-buffer** for single-producer, multiple-consumer (SPMC) latest-value updates.

Internals:
- Two `T` buffers, `buffers_[0]` and `buffers_[1]`
- `readIdx_` atomic — which buffer is current
- `version_` atomic — increments on each publish

API:
- `publish(msg)` — write to the inactive buffer, then atomically swap `readIdx_`
- `tryRead(out, lastVersion)` — if version changed since last read, copy the current buffer into `out` and update `lastVersion`; returns false if nothing new

This design means readers **never block the writer** and always get the latest complete value.  
Multiple readers can read independently without contention.

Used for: `world.snapshots` (simulation loop/WorldManager → Renderer + HapticEngine in the current `main.cpp` wiring).

---

## MessageBus

`MessageBus` (`include/messaging/MessageBus.h`) is a named channel registry.

API:
- `channel<T>(name)` — get or create a `Channel<T>` by name
- `snapshot<T>(name)` — get or create a `SnapshotChannel<T>` by name

Channels are stored as `unique_ptr<ChannelBase>` in an `unordered_map<string, ...>`.  
If the same name is requested with a different type or kind, it throws.

`MessageBus` is created once in `main.cpp` and subsystems receive references to specific channels — they don't hold a reference to the bus itself.

---

## Named Channels in Use

| Name                    | Type                             | Producer         | Consumer(s)                             |
| ----------------------- | -------------------------------- | ---------------- | --------------------------------------- |
| `world.commands`        | `Channel<WorldCommand>`          | UI/render        | `WorldManager`                          |
| `world.snapshots`       | `SnapshotChannel<WorldSnapshot>` | simulation loop  | `GlSceneRenderer`, `HapticEngine`       |
| `haptics.tool_in`       | `Channel<ToolStateMsg>`          | mouse/debug path | viewport debug path, optional PhysX path |
| `haptics.snapshots`     | `Channel<HapticSnapshotMsg>`     | `HapticEngine`   | `GlSceneRenderer` / viewport path       |
| `haptics.wrenches`      | `Channel<HapticWrenchCmd>`       | `HapticEngine`   | `PhysicsEnginePhysX`                    |
| `device.tool_in`        | `Channel<ToolStateMsg>`          | `DeviceAdapter`  | `HapticEngine`                          |
| `device.wrench_cmd`     | `Channel<HapticWrenchCmd>`       | `HapticEngine`   | `DeviceAdapter`                         |
| `logging.device_timing` | `Channel<DeviceTimingLogMsg>`    | `DeviceAdapter`  | log thread                              |
| `logging.device_state`  | `Channel<DeviceStateLogMsg>`     | `DeviceAdapter`  | log thread                              |
| `logging.sim_validation` | `Channel<SimulationValidationLogMsg>` | `HapticEngine` | log thread                              |
| `physics.haptics_wrenches` | `Channel<HapticWrenchCmd>`    | reserved PhysX output | none active in current code        |

In the current `main.cpp`, `HapticEngine` is wired to `device.tool_in` for the physical device path. `haptics.tool_in` remains available for the software mouse/debug path, but it is not the live haptic input unless the constructor wiring is changed.

See [[Thread_Message_Bus_Diagram]] for the thread-level block diagram and a payload-by-payload summary.

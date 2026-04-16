# Messaging

Part of the [[Implementation_Index]].

## Quick Navigation

- [[#Overview]]
- [[#Channel (Queue)]]
- [[#SnapshotChannel (Latest-Value)]]
- [[#MessageBus]]
- [[#Named Channels in Use]]

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

`Channel<T>` (`Rewrite/include/messaging/Channel.h`) is a mutex-protected `std::queue<T>`.

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

`SnapshotChannel<T>` (`Rewrite/include/messaging/SnapshotChannel.h`) is a **lock-free double-buffer** for single-producer, multiple-consumer (SPMC) latest-value updates.

Internals:
- Two `T` buffers, `buffers_[0]` and `buffers_[1]`
- `readIdx_` atomic — which buffer is current
- `version_` atomic — increments on each publish

API:
- `publish(msg)` — write to the inactive buffer, then atomically swap `readIdx_`
- `tryRead(out, lastVersion)` — if version changed since last read, copy the current buffer into `out` and update `lastVersion`; returns false if nothing new

This design means readers **never block the writer** and always get the latest complete value.  
Multiple readers (renderer + physics) can read independently without contention.

Used for: `world.snapshots` (WorldManager → Renderer + Physics).

---

## MessageBus

`MessageBus` (`Rewrite/include/messaging/MessageBus.h`) is a named channel registry.

API:
- `channel<T>(name)` — get or create a `Channel<T>` by name
- `snapshot<T>(name)` — get or create a `SnapshotChannel<T>` by name

Channels are stored as `unique_ptr<ChannelBase>` in an `unordered_map<string, ...>`.  
If the same name is requested with a different type or kind, it throws.

`MessageBus` is created once in `main.cpp` and subsystems receive references to specific channels — they don't hold a reference to the bus itself.

---

## Named Channels in Use

| Name | Type | Producer | Consumer(s) |
|---|---|---|---|
| `world.commands` | `Channel<WorldCommand>` | UI/any | `WorldManager` |
| `world.snapshots` | `SnapshotChannel<WorldSnapshot>` | `WorldManager` | `GlSceneRenderer`, `PhysicsEnginePhysX` |
| `haptics.tool_in` | `Channel<ToolStateMsg>` | (mouse fallback) | `HapticEngine` |
| `haptics.snapshots` | `Channel<HapticSnapshotMsg>` | `HapticEngine` | `GlSceneRenderer` |
| `haptics.wrenches` | `Channel<HapticWrenchCmd>` | `HapticEngine` | `PhysicsEnginePhysX` |
| `device.tool_in` | `Channel<ToolStateMsg>` | `DeviceAdapter` | `HapticEngine` |
| `device.wrench_cmd` | `Channel<HapticWrenchCmd>` | `HapticEngine` | `DeviceAdapter` |
| `logging.device_timing` | `Channel<DeviceTimingLogMsg>` | `DeviceAdapter` | log thread |
| `logging.device_state` | `Channel<DeviceStateLogMsg>` | `DeviceAdapter` | log thread |

In `main.cpp`, `haptics.tool_in` is used for a software mouse-based tool input, and `device.tool_in` is used when the physical device is connected — both feed `HapticEngine` depending on configuration.

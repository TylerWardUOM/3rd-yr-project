# System Implementation

Top-level index for the system implementation notes.  
Written to support the system implementation section of the project report.

## Architecture Overview

The system is multi-threaded.  Each major concern runs independently and communicates via the [[Messaging]] layer.

```
Main Thread          Simulation Thread     Haptic Thread        Device Thread
─────────────        ─────────────────     ─────────────        ─────────────
GlSceneRenderer      WorldManager          HapticEngine         DeviceAdapter
  + UI               PhysicsEnginePhysX      contact / proxy      SerialLink
  + Camera             step (30 Hz)           loop (1 kHz)         I/O (1 kHz)
  render loop
```

### Shared channels (from [[Messaging]])

| Channel name | Type | Direction |
|---|---|---|
| `world.commands` | Queue | UI/any → WorldManager |
| `world.snapshots` | Snapshot | WorldManager → Renderer, Physics |
| `haptics.tool_in` | Queue | DeviceAdapter → HapticEngine |
| `haptics.snapshots` | Queue | HapticEngine → Renderer |
| `haptics.wrenches` | Queue | HapticEngine → Physics |
| `device.wrench_cmd` | Queue | HapticEngine → DeviceAdapter |

---

## Notes

- [[Rendering Engine]] — OpenGL renderer, window, shaders, camera, UI
- [[World System]] — WorldManager, objects, commands, snapshots, geometry
- [[Physics Engine]] — PhysX integration, actors, step pipeline
- [[Haptic Engine]] — contact search (SDF), proxy projection, virtual coupling
- [[Hardware Interface]] — SerialLink, DeviceAdapter, packets, kinematics/torques
- [[Messaging]] — Channel, SnapshotChannel, MessageBus

---

## Application Entry Point

`src/main.cpp` wires all subsystems together:

1. Create shared infrastructure (`GeometryDatabase`, `RenderMeshRegistry`, `MessageBus`)
2. Create all named channels on the bus
3. Create `WorldManager`, `Window`, `GlSceneRenderer`, `HapticEngine`, `DeviceAdapter`, `PhysicsEnginePhysX`
4. Register initial scene objects (plane, sphere, cube)
5. Spawn threads: sim thread, haptics thread, device thread, log thread
6. Render loop on main thread
7. On window close: stop flags, join threads, write timing CSVs

This is the composition root — nothing is hard-wired inside subsystems; dependencies are injected at startup.

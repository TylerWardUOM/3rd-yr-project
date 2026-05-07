# Thread Message-Bus Diagram

Part of the [[Implementation_Index]] and [[Messaging]] notes.

This diagram shows the runtime threads created in `src/main.cpp` and the named
message-bus channels passed between them.

```mermaid
flowchart LR
    %% ------------------------------------------------------------------
    %% Threads
    %% ------------------------------------------------------------------
    subgraph render["Main / render thread"]
        renderer["GlSceneRenderer<br/>Window + ImGui + viewport"]
        viewport["ViewportController<br/>mouse debug tool pose"]
    end

    subgraph sim["Simulation thread"]
        simloop["simulationLoop<br/>steps world + PhysX<br/>publishes snapshots"]
        wm["WorldManager<br/>drains commands<br/>authoritative world state"]
        physx["PhysicsEnginePhysX<br/>240 Hz fixed PhysX substeps"]
    end

    subgraph haptic["Haptic thread"]
        he["HapticEngine<br/>1 kHz SDF contact<br/>virtual coupling"]
    end

    subgraph device["Device thread"]
        da["DeviceAdapter<br/>1 kHz serial bridge<br/>FK + Jacobian torque map"]
    end

    subgraph log["Log thread"]
        logger["CSV log buffers<br/>written on shutdown"]
    end

    mcu["STM32 firmware<br/>state packets / torque packets"]

    %% ------------------------------------------------------------------
    %% Bus channels
    %% ------------------------------------------------------------------
    wc["world.commands<br/>Channel&lt;WorldCommand&gt;<br/>create, remove, edit, physics props"]
    ws["world.snapshots<br/>SnapshotChannel&lt;WorldSnapshot&gt;<br/>seq, sim time, object poses, roles, physics props"]
    hti["haptics.tool_in<br/>Channel&lt;ToolStateMsg&gt;<br/>mouse/debug tool pose and timestamp"]
    dti["device.tool_in<br/>Channel&lt;ToolStateMsg&gt;<br/>device FK pose, optional velocity, timestamp"]
    hs["haptics.snapshots<br/>Channel&lt;HapticSnapshotMsg&gt;<br/>device pose, proxy pose, force, timestamp"]
    hw["haptics.wrenches<br/>Channel&lt;HapticWrenchCmd&gt;<br/>target object, contact point, -force impulse, duration"]
    dw["device.wrench_cmd<br/>Channel&lt;HapticWrenchCmd&gt;<br/>device force command, zero when no contact"]
    lt["logging.device_timing<br/>Channel&lt;DeviceTimingLogMsg&gt;<br/>matched rx/tx timestamps, force, torques, saturation"]
    ls["logging.device_state<br/>Channel&lt;DeviceStateLogMsg&gt;<br/>raw parsed state packets, joint angles, applied torques, watchdog"]
    lv["logging.sim_validation<br/>Channel&lt;SimulationValidationLogMsg&gt;<br/>device/proxy positions, force, normal, penetration, contact flag"]
    phw["physics.haptics_wrenches<br/>Channel&lt;HapticWrenchCmd&gt;<br/>reserved PhysX-to-haptics contact wrench output"]

    %% ------------------------------------------------------------------
    %% Active message paths
    %% ------------------------------------------------------------------
    renderer --> wc --> wm
    simloop --> wm
    simloop --> physx
    physx -. "direct pose write-back" .-> wm
    simloop --> ws
    ws --> renderer
    ws --> he

    viewport --> hti
    hti -. "debug / optional tool input" .-> physx
    hti -. "currently not the live HapticEngine input" .-> viewport

    da --> dti --> he
    he --> hs --> renderer
    he --> hw --> physx
    he --> dw --> da

    da --> lt --> logger
    da --> ls --> logger
    he --> lv --> logger

    da <--> mcu
    physx -. "constructed, not actively published in current code" .-> phw

    %% ------------------------------------------------------------------
    %% Styling
    %% ------------------------------------------------------------------
    classDef thread fill:#f7f7f7,stroke:#555,stroke-width:1px,color:#111;
    classDef queue fill:#e7f0ff,stroke:#2b5fab,stroke-width:1px,color:#111;
    classDef snapshot fill:#e8f7ea,stroke:#2f7d3b,stroke-width:1px,color:#111;
    classDef external fill:#fff4df,stroke:#9a6b1f,stroke-width:1px,color:#111;

    class renderer,viewport,simloop,wm,physx,he,da,logger thread;
    class wc,hti,dti,hs,hw,dw,lt,ls,lv,phw queue;
    class ws snapshot;
    class mcu external;
```

## Channel Summary

| Channel name | Bus type | Producer thread | Consumer thread(s) | Payload carried |
| --- | --- | --- | --- | --- |
| `world.commands` | `Channel<WorldCommand>` | Main/render | Simulation | Scene mutation requests: create/remove/edit object and set/patch physics properties. |
| `world.snapshots` | `SnapshotChannel<WorldSnapshot>` | Simulation | Main/render, haptic | Latest world state: sequence, simulation time, object ids, geometry ids, poses, velocities, roles, colours, physics properties. |
| `haptics.tool_in` | `Channel<ToolStateMsg>` | Main/render viewport | Main/render viewport, optional PhysX path | Mouse/debug tool pose, optional velocity/button, timestamp. In the current `main.cpp`, this is not the live input passed to `HapticEngine`. |
| `device.tool_in` | `Channel<ToolStateMsg>` | Device | Haptic | Tool pose from device forward kinematics, optional velocity/button, timestamp. This is the live haptic input in the current `main.cpp`. |
| `haptics.snapshots` | `Channel<HapticSnapshotMsg>` | Haptic | Main/render | Device pose, proxy pose, displayed contact force, timestamp. |
| `haptics.wrenches` | `Channel<HapticWrenchCmd>` | Haptic | Simulation / PhysX | Contact wrench for the simulated object: target id, world force, torque, contact point, duration, timestamp. The force is `-F` so the object receives the reaction impulse. |
| `device.wrench_cmd` | `Channel<HapticWrenchCmd>` | Haptic | Device | Force command sent to the physical device path. Carries `+F` in contact and an explicit zero force out of contact. |
| `logging.device_timing` | `Channel<DeviceTimingLogMsg>` | Device | Log | Matched closed-loop timing: rx parse, tool publish, wrench consume, tx start/done, sequence ids, joint angles, force, raw/clamped torques, saturation flags. |
| `logging.device_state` | `Channel<DeviceStateLogMsg>` | Device | Log | Raw parsed firmware state packets: chunk/read timestamps, sequence ids, MCU time, joint angles, applied torques, watchdog and saturation flags. |
| `logging.sim_validation` | `Channel<SimulationValidationLogMsg>` | Haptic | Log | Validation samples: device/proxy positions, force, contact normal, penetration depth, signed SDF distance, contact flag. |
| `physics.haptics_wrenches` | `Channel<HapticWrenchCmd>` | Reserved in PhysX | None active | Constructed and passed to `PhysicsEnginePhysX` as an optional PhysX-to-haptics output, but no active publish path exists in the current code. |

## Bus Type Notes

`Channel<T>` is a mutex-protected FIFO queue. It preserves every message until a
consumer drains or consumes it, which makes it suitable for commands, wrenches,
tool samples, and logs.

`SnapshotChannel<T>` is a double-buffered latest-value channel. It does not queue
history; readers get the newest complete `WorldSnapshot` if its version changed,
which is why it is used for high-frequency world state fan-out.

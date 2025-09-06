# 3rd-yr-project

Goal: build a small haptic training interface for a (simulated first) robotic manipulator. Cheap-ish prototype: encoders + motor(s) pushing back, loop talks to a sim (start simple, upgrade later). Focus on feel + low latency more than pretty graphics.

Why: haptics help users learn manipulator control faster vs pure visuals; sim keeps hardware risk = zero early.

Core bits (initial):
- Device: position sensing + motor torque
- MCU firmware: fast loop, packet encode/decode
- Host side: simple sim / placeholder physics, force calc, send commands back
- Feedback: basic spring/damper, collision resistance, soft limits
- Logging: latency + jitter so I don’t guess

Targets (rough): <10 ms device loop, <30 ms end-to-end path. PD first, add I only if needed. Safety: watchdog + torque caps.

Planned rough phases (will shift):
1) Interfaces + packet draft
2) Hardware bring-up (encoders streaming)
3) Actuation + basic force render (1‑DOF)
4) Hook to simple sim (even toy model)
5) Improve feel (tuning, soften limits)
6) Docs + quick eval
7) (Maybe) swap sim adapter for real arm stub

Tech/options on radar: STM32 or RP2040, Python host first, later ROS2/DQ Robotics if multi-DOF. Sim: maybe Gazebo or Isaac Sim once basics proven.

Stretch ideas (only if time): vibration cues, trajectory “ghost”, adaptive training, nicer visualization, multi-DOF handle.

(Will evolve; see project_proposal.md + notes folder for the structured version and brain dumps.)
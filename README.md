# Real-Time Haptic Rendering System

**Repository:** https://github.com/TylerWardUOM/3rd-yr-project

---

## Introduction

This project implements a real-time haptic rendering system that allows a user to physically feel virtual 3D objects through a custom-built 2-DOF planar robot arm. The system tracks the position of the device's end-effector and uses that as the position of a virtual "tool" inside a simulated 3D environment. When the tool touches a virtual object, a force is calculated and fed back to the physical device — giving the user the sensation of actually pushing against something.

The software is written entirely in C++17 and runs on Windows. It brings together four real-time concerns — rendering, physics simulation, haptic force computation, and hardware communication — each running on its own thread and communicating through a shared message-bus. The rendering thread provides a live OpenGL 3D view of the scene with an ImGui control panel. The physics thread runs NVIDIA PhysX at 240 Hz for rigid-body dynamics. The haptic thread runs at 1 kHz to compute contact forces using Signed Distance Functions (SDFs). The device thread also runs at 1 kHz, reading joint angles from the robot over serial and sending torque commands back.

---

## System Overview

```
+--------------------------------------------------------------------+
|                         Application                                |
|                                                                    |
|  Main Thread      Sim Thread        Haptic Thread   Device Thread  |
|  -----------      ----------        ------------    -------------  |
|  GlSceneRenderer  WorldManager      HapticEngine    DeviceAdapter  |
|  + ImGui UI       PhysicsEnginePhysX   (1 kHz)      SerialLink     |
|  + Camera         (30 Hz / 240 Hz sub-step)          (1 kHz)       |
+--------------------------------------------------------------------+

Message Bus channels:
  world.snapshots    -->  Renderer, Physics
  haptics.tool_in    -->  HapticEngine
  haptics.wrenches   -->  PhysicsEngine
  device.wrench_cmd  -->  DeviceAdapter
  haptics.snapshots  -->  Renderer (overlay)
```

A demo video of early kinematics testing is available in the repository at:
`docs/Media/Kinematics Demo 2025-09-20.mp4`
> Note: Still need to get a better more upto date video

---

## Installation Instructions

These instructions assume a clean Windows PC with no prior project dependencies installed.

### 1. Install Prerequisites

| Tool | Where to get it |
|---|---|
| Visual Studio 2022 (with C++ Desktop workload) | https://visualstudio.microsoft.com/downloads/ |
| CMake 3.20 or newer | https://cmake.org/download/ |
| Git | https://git-scm.com/downloads |
| vcpkg | https://github.com/microsoft/vcpkg |

**Install vcpkg** (package manager for C++ libraries):

    git clone https://github.com/microsoft/vcpkg.git C:\Users\<you>\vcpkg
    cd C:\Users\<you>\vcpkg
    bootstrap-vcpkg.bat

**Install PhysX via vcpkg:**

    vcpkg install physx:x64-windows

### 2. Clone the Repository

    git clone --recurse-submodules https://github.com/TylerWardUOM/3rd-yr-project.git
    cd 3rd-yr-project

If you already cloned without submodules, run:

    git submodule update --init --recursive

This pulls in the vendored `imgui` and `glm` libraries automatically.

### 3. Update the vcpkg Path

Open `CMakeLists.txt` and change this line to match where your vcpkg is installed:

    set(VCPKG_INSTALLED_DIR "C:/Users/<you>/vcpkg/installed/x64-windows")

### 4. Configure and Build

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

This produces two executables inside the build folder:

- `app.exe` — the main haptic rendering application
- `serial_tests.exe` — standalone serial communication test tool

### 5. Hardware (Optional)

To use the physical haptic device you will also need:

- A 2-DOF planar robot arm with quadrature encoders and BLDC motors, connected by USB serial
- The matching firmware flashed to the device's MCU (not included in this repo)
- COM port access (default: COM4, 460800 baud — configurable in `src/main.cpp`)

The application will run without the hardware connected; it will fall back to mouse-based tool control.

---

## How to Run

### Main Application

    cd build\Release
    app.exe

A window opens showing a 3D scene with a ground plane, a sphere, and a cube.

**Controls:**

| Input | Action |
|---|---|
| Right-click + drag | Look around (mouse-look) |
| W / A / S / D | Fly camera |
| Scroll wheel | Zoom |
| ImGui panel (right side) | Select and move objects, change physics properties, adjust camera |

If the physical device is connected on COM4 at 460800 baud, a green sphere (device end-effector) and a blue sphere (haptic proxy) will appear in the scene as overlays.

### Serial Communication Tests

The `serial_tests` executable lets you measure serial link performance without running the full simulation.

**Packet rate / integrity test** (checks packets received per second):

    serial_tests rate COM4 460800 30 rate_results.csv

**Round-trip time (RTT) test** (measures latency):

    serial_tests rtt COM4 460800 1000 2 rtt_results.csv

Arguments: `<port> <baud> <duration_sec or count> <interval_ms> <output_csv>`

Results are written to CSV for analysis.

---

## Technical Details

### Haptic Contact Detection — Signed Distance Functions (SDF)

Each virtual object's surface is represented by a Signed Distance Function φ(x). The value is positive outside the surface, zero on it, and negative inside. The gradient of φ points along the outward surface normal.

**Plane SDF** — for a plane with unit normal n and origin distance d:

    φ(x) = n · x − d

**Sphere SDF** — for a sphere with centre c and radius r:

    φ(x) = |x − c| − r

**Proxy projection** — when the tool penetrates a surface (φ < 0), a haptic proxy is snapped to the nearest point on the surface:

    x_proxy = x_tool − φ(x_tool) · n̂

This is the *god-object / proxy* method for haptic rendering (Zilles & Salisbury, 1995).

---

### Virtual Coupling (Force Feedback)

A virtual spring-damper couples the physical device position x_d to the constrained proxy position x_p. This produces the force fed back to the device:

    F_cpl = K(x_p − x_d) + D(ẋ_p − ẋ_d)

Parameters used in this implementation:

| Parameter | Value | Description |
|---|---|---|
| K | 500 N/m | Spring stiffness |
| M | 0.02 kg | Virtual mass (for critical damping calc) |
| D | 0.7 × 2√(KM) ≈ 8.9 N·s/m | ~70% of critical damping, for stability |
| F_max | 31 N | Hard force clamp — device safety limit |

The equal and opposite reaction force (−F) is applied as an impulse to the corresponding PhysX rigid body each physics step, so virtual objects can be pushed around by the device.

---

### Forward Kinematics

The physical device is a 2-DOF planar robot arm with equal link lengths L1 = L2 = 0.15 m. Joint angles q1 and q2 (in radians) are read from the encoders. The end-effector position is:

    x = L1·cos(q1) + L2·cos(q1 + q2)
    y = L1·sin(q1) + L2·sin(q1 + q2)

The end-effector orientation quaternion is a rotation of (q1 + q2) about the Z axis.

---

### Jacobian Torque Computation

To render a Cartesian force F = [Fx, Fy] at the end-effector, joint torques are computed via the Jacobian transpose (τ = Jᵀ · F):

    J = [ −L1·sin(q1) − L2·sin(q1+q2),   −L2·sin(q1+q2) ]
        [  L1·cos(q1) + L2·cos(q1+q2),    L2·cos(q1+q2)  ]

    τ1 = J11·Fx + J21·Fy
    τ2 = J12·Fx + J22·Fy

These torques are packed into a `TorqueCommandPacket` (with header `0xAA 0x55` and a checksum) and sent over serial to the MCU.

---

### Threading Architecture

| Thread | Task | Rate |
|---|---|---|
| Main | OpenGL render loop, ImGui, camera | Display refresh |
| Simulation | WorldManager step, PhysX step | ~30 Hz (1/240 s substeps) |
| Haptics | SDF contact search, proxy update, force computation | 1 kHz |
| Device | Serial read/parse, FK, torque send | 1 kHz |

All inter-thread communication uses lock-free message bus channels (`Channel<T>` for queues, `SnapshotChannel<T>` for latest-value reads).

---

### Logging

On shutdown, the application writes two CSV files to the working directory:

- `device_timing.csv` — per-cycle timestamps (rx parse, tool publish, wrench consume, serial write) and signal values (q1, q2, fx, fy, τ1, τ2)
- `device_state_log.csv` — every parsed state packet from the device (sequence number, MCU timestamp, joint angles)

These are useful for diagnosing latency and communication issues.

---

## Known Issues / Future Improvements

- **Windows-only:** `SerialLink` uses the Win32 `HANDLE` API. Porting to Linux/macOS would require replacing this with a POSIX serial implementation.
- **Hardcoded COM port:** The device port (`COM4`) and baud rate (`460800`) are hardcoded in `src/main.cpp`. These should be made configurable via a config file or command-line argument.
- **Hardcoded vcpkg path:** `CMakeLists.txt` has an absolute path to the PhysX install location. This should be replaced with a proper CMake toolchain file or `find_package` call.
- **No haptic loop stop flag:** `HapticEngine::run()` has no stop flag — the thread is detached rather than joined on shutdown. A clean shutdown flag should be added.
- **2D device only:** The current hardware is limited to a planar (XY) workspace. Extending to a 3-DOF or 6-DOF device would require updated kinematics and Jacobian.
- **No friction / texture rendering:** Only normal (penetration) forces are currently rendered. Surface friction, texture variation, and damping variation across surfaces are not yet implemented.
- **Mouse control limited:** Mouse-based tool control (fallback without the device) does not provide velocity data, so the damping term in the virtual coupling force is zero in that mode.
- **Actor rebuild is a full rebuild:** Any topology or physics property change causes all PhysX actors to be recreated from scratch. Incremental updates would be more efficient for larger scenes.

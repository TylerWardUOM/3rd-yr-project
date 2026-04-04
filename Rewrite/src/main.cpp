#include "world/WorldManager.h"
#include "geometry/GeometryDatabase.h"
#include "geometry/GeometryEntry.h"
#include "geometry/GeometryFactory.h"
#include "render/Camera.h"
#include "platform/Window.h"
#include "render/ISceneRenderer.h"
#include "render/GlSceneRenderer.h"
#include "render/RenderMeshRegistry.h"
#include "data/Commands.h"
#include "messaging/Channel.h"
#include "messaging/SnapshotChannel.h"
#include "messaging/MessageBus.h"
#include "engines/HapticEngine.h"
#include "engines/PhysicsEnginePhysX.h"
#include "hardware/DeviceAdapter.h"
#include "data/LogMessages.h"

#include <thread>
#include <atomic>
#include <fstream>
#include <vector>
#include <chrono>
#include <iostream>

void simulationLoop(
    WorldManager& wm,
    PhysicsEnginePhysX& physics,
    msg::SnapshotChannel<WorldSnapshot>& worldSnaps,
    std::atomic<bool>& running
) {
    const double maxDt = 1.0 / 30.0;
    auto prev = std::chrono::steady_clock::now();

    while (running.load(std::memory_order_relaxed)) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - prev).count();
        prev = now;

        dt = std::min(dt, maxDt);

        wm.step(dt);
        physics.step(dt);

        WorldSnapshot snapshot = wm.buildSnapshot();
        worldSnaps.publish(snapshot);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    // ------------------------------------------------------------
    // Core systems
    // ------------------------------------------------------------
    GeometryDatabase geomDb;
    RenderMeshRegistry meshRegistry;
    GeometryFactory geomFactory(geomDb, meshRegistry);

    msg::MessageBus bus;

    auto& worldCmds     = bus.channel<WorldCommand>("world.commands");
    auto& worldSnaps    = bus.snapshot<WorldSnapshot>("world.snapshots");
    auto& toolIn        = bus.channel<ToolStateMsg>("haptics.tool_in");
    auto& hapticOut     = bus.channel<HapticSnapshotMsg>("haptics.snapshots");
    auto& wrenchOut     = bus.channel<HapticWrenchCmd>("haptics.wrenches");
    auto& deviceIn      = bus.channel<ToolStateMsg>("device.tool_in");
    auto& deviceCmdOut  = bus.channel<HapticWrenchCmd>("device.wrench_cmd");
    auto& timingLog     = bus.channel<DeviceTimingLogMsg>("logging.device_timing");

    std::vector<DeviceTimingLogMsg> timingLogs;

    WorldManager wm(geomDb, geomFactory, worldCmds);

    Window win({});
    GlSceneRenderer renderer(win, geomDb, meshRegistry, worldCmds, toolIn, hapticOut, worldSnaps);

    HapticEngine haptics(
        geomDb,
        worldSnaps,
        deviceIn,       // use toolIn for mouse control, deviceIn for real device input
        hapticOut,
        wrenchOut,
        deviceCmdOut
    );

    DeviceAdapter deviceAdapter(deviceIn, deviceCmdOut, timingLog);

    PhysicsEnginePhysX physics(
        wm,
        geomDb,
        wrenchOut,
        toolIn,
        bus.channel<HapticWrenchCmd>("physics.haptics_wrenches")
    );
    physics.setFixedDt(1.0 / 240.0);

    // ------------------------------------------------------------
    // Create initial objects
    // ------------------------------------------------------------
    GeometryID plane  = geomFactory.getPlane();
    GeometryID sphere = geomFactory.getSphere();
    GeometryID cube   = geomFactory.getCube();

    CreateObjectCommand cmd{plane};
    cmd.initialPose.s = 25.0f;
    wm.apply(WorldCommand{cmd});
    wm.apply(WorldCommand{CreateObjectCommand{
        sphere,
        Pose{{2.0, 5.0, 0.0}, {0, 0, 0, 1}, 0.2f},
        {0.2f, 0.2f, 0.8f}
    }});
    wm.apply(WorldCommand{CreateObjectCommand{
        cube,
        Pose{{-1.0, 1.0, 0.0}, {0, 0, 0, 1}, 1.0f},
        {0.8f, 0.2f, 0.2f}
    }});

    // ------------------------------------------------------------
    // Thread running flags
    // ------------------------------------------------------------
    std::atomic<bool> simRunning{true};
    std::atomic<bool> appRunning{true};
    std::atomic<bool> logRunning{true};

    // ------------------------------------------------------------
    // Threads
    // ------------------------------------------------------------
    std::thread simThread(
        simulationLoop,
        std::ref(wm),
        std::ref(physics),
        std::ref(worldSnaps),
        std::ref(simRunning)
    );

    std::thread hapticsThread(&HapticEngine::run, &haptics);

    if (!deviceAdapter.connect("COM4", 460800)) {
        std::cerr << "Failed to connect device on COM4\n";
    }

    std::thread deviceThread([&]() {
        while (appRunning.load(std::memory_order_relaxed)) {
            auto now = std::chrono::steady_clock::now();

            deviceAdapter.update(
                std::chrono::duration<double>(now.time_since_epoch()).count()
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::thread logThread([&]() {
        while (logRunning.load(std::memory_order_relaxed) || timingLog.size() > 0) {
            DeviceTimingLogMsg msg;
            bool gotAny = false;

            while (timingLog.tryConsume(msg)) {
                timingLogs.push_back(msg);
                gotAny = true;
            }

            if (!gotAny) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });

    // ------------------------------------------------------------
    // Render loop (main thread)
    // ------------------------------------------------------------
    while (win.isOpen()) {
        renderer.render();
    }

    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------
    simRunning.store(false, std::memory_order_relaxed);
    appRunning.store(false, std::memory_order_relaxed);
    logRunning.store(false, std::memory_order_relaxed);

    if (simThread.joinable()) {
        simThread.join();
    }

    if (deviceThread.joinable()) {
        deviceThread.join();
    }

    // HapticEngine currently has no stop flag in this version, so still detached.
    // Ideally you should add a running flag inside HapticEngine::run() and join it too.
    if (hapticsThread.joinable()) {
        hapticsThread.detach();
    }

    if (logThread.joinable()) {
        logThread.join();
    }

    // ------------------------------------------------------------
    // Write timing CSV after logger has finished
    // ------------------------------------------------------------
    {
        std::ofstream csv("device_timing.csv");
        csv << "rx_seq,t_rx_parse_ns,t_tool_publish_ns,t_wrench_consume_ns,"
               "t_tx_start_ns,t_tx_done_ns,q1,q2,fx,fy,tau1,tau2\n";

        for (const auto& x : timingLogs) {
            csv << x.rx_seq << ","
                << x.t_rx_parse_ns << ","
                << x.t_tool_publish_ns << ","
                << x.t_wrench_consume_ns << ","
                << x.t_tx_start_ns << ","
                << x.t_tx_done_ns << ","
                << x.q1 << ","
                << x.q2 << ","
                << x.fx << ","
                << x.fy << ","
                << x.tau1 << ","
                << x.tau2 << "\n";
        }
    }

    return 0;
}
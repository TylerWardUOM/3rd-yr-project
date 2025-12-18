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

#include <thread>
#include <atomic>


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

        // Optional: avoid busy-spin
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

    auto& worldCmds  = bus.channel<WorldCommand>("world.commands");
    auto& worldSnaps = bus.snapshot<WorldSnapshot>("world.snapshots");
    auto& toolIn     = bus.channel<ToolStateMsg>("haptics.tool_in");
    auto& hapticOut  = bus.channel<HapticSnapshotMsg>("haptics.snapshots");
    auto& wrenchOut  = bus.channel<HapticWrenchCmd>("haptics.wrenches");

    WorldManager wm(geomDb, geomFactory, worldCmds);

    Window win({});
    GlSceneRenderer renderer(win, geomDb, meshRegistry, worldCmds, toolIn, hapticOut, worldSnaps);

    HapticEngine haptics(
        geomDb,
        worldSnaps,
        toolIn,
        hapticOut,
        wrenchOut
    );

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

    CreateObjectCommand cmd{plane};
    //cmd.dynamic = false;
    cmd.initialPose.s = 25.0f;
    wm.apply(WorldCommand{cmd});
    wm.apply(WorldCommand{CreateObjectCommand{sphere, Pose{{0.0,5.0,0.0},{0,0,0,1}, 0.2f}, {0.2f,0.2f,0.8f}}});


    std::atomic<bool> simRunning{true};

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

    // ------------------------------------------------------------
    // Render loop (main thread)
    // ------------------------------------------------------------
    WorldSnapshot latest;

    while (win.isOpen()) {
        // Non-blocking read
        renderer.render();
    }

    simRunning.store(false, std::memory_order_relaxed);
    simThread.join();
    hapticsThread.detach();
}

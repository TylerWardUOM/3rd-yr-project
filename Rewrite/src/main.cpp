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
#include "messaging/MessageBus.h"
#include "engines/HapticEngine.h"
#include "engines/PhysicsEnginePhysX.h"

#include <thread>

int main() {
    // ------------------------------------------------------------
    // Core shared systems
    // ------------------------------------------------------------
    GeometryDatabase geomDb;
    RenderMeshRegistry meshRegistry;
    GeometryFactory geomFactory(geomDb, meshRegistry);

    msg::MessageBus bus;

    // ------------------------------------------------------------
    // Channels
    // ------------------------------------------------------------
    auto& worldCmds   = bus.channel<WorldCommand>("world.commands");
    auto& worldSnaps  = bus.channel<WorldSnapshot>("world.snapshots");
    auto& toolIn      = bus.channel<ToolStateMsg>("haptics.tool_in");
    auto& hapticOut   = bus.channel<HapticSnapshotMsg>("haptics.snapshots");
    auto& wrenchOut   = bus.channel<HapticWrenchCmd>("haptics.wrenches");

    // ------------------------------------------------------------
    // Engines
    // ------------------------------------------------------------
    WorldManager wm(geomDb, geomFactory, worldCmds);

    Window win({});
    GlSceneRenderer renderer(win, geomDb, meshRegistry, worldCmds, toolIn, hapticOut);

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

    // Optional: connect to real device (safe even if dummy)
    // haptics.connectDevice("COM3", 115200);

    // Start haptics thread (1 kHz loop)
    std::thread hapticsThread(&HapticEngine::run, &haptics);

    // ------------------------------------------------------------
    // Create initial objects
    // ------------------------------------------------------------
    GeometryID plane  = geomFactory.getPlane();
    GeometryID sphere = geomFactory.getSphere();

    CreateObjectCommand cmd{plane};
    //cmd.dynamic = false;
    wm.apply(WorldCommand{cmd});
    wm.apply(WorldCommand{CreateObjectCommand{sphere, Pose{{0.0,5.0,0.0},{0,0,0,1}, 0.2f}, {0.2f,0.2f,0.8f}}});


    // ------------------------------------------------------------
    // Main loop (render + world)
    // ------------------------------------------------------------

    const double dt = 1.0 / 60.0;
    physics.setFixedDt(1.0 / 240.0); // 240 Hz physics
    auto prev = std::chrono::steady_clock::now();

    while (win.isOpen()) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - prev).count();
        prev = now;

        // clamp to avoid huge jumps (debugger pauses etc.)
        dt = std::min(dt, 1.0/30.0);

        wm.step(dt);
        physics.step(dt);

        WorldSnapshot snapshot = wm.buildSnapshot();
        worldSnaps.publish(snapshot);
        renderer.render(snapshot);
    }


    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------
    hapticsThread.detach(); // or implement a clean stop flag
    return 0;
}

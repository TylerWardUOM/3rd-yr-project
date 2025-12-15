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

    // Optional: connect to real device (safe even if dummy)
    // haptics.connectDevice("COM3", 115200);

    // Start haptics thread (1 kHz loop)
    std::thread hapticsThread(&HapticEngine::run, &haptics);

    // ------------------------------------------------------------
    // Create initial objects
    // ------------------------------------------------------------
    GeometryID plane  = geomFactory.getPlane();
    GeometryID sphere = geomFactory.getSphere();

    wm.apply(WorldCommand{CreateObjectCommand{plane}});
    wm.apply(WorldCommand{CreateObjectCommand{sphere, Pose{{0.0,0.5,0.0},{0,0,0,1}, 0.2f}, {0.2f,0.2f,0.8f}}});


    // ------------------------------------------------------------
    // Main loop (render + world)
    // ------------------------------------------------------------
    while (win.isOpen()) {

        const double dt = 1.0 / 60.0;

        // Build + publish world snapshot
        WorldSnapshot snapshot = wm.buildSnapshot();
        worldSnaps.publish(snapshot);

        // Render (still direct, no messaging needed yet)
        renderer.render(snapshot);

        // Apply world commands
        wm.step(dt);

        
    }

    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------
    hapticsThread.detach(); // or implement a clean stop flag
    return 0;
}

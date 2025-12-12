#include "world/WorldManager.h"
#include "geometry/GeometryDatabase.h"
#include "geometry/GeometryEntry.h" 
#include "geometry/GeometryFactory.h"
#include "render/Camera.h"
#include "platform/Window.h"
#include "render/ISceneRenderer.h"
#include "render/GlSceneRenderer.h"
#include "render/RenderMeshRegistry.h"

GeometryDatabase geomDb;
RenderMeshRegistry meshRegistry;
GeometryFactory geomFactory(geomDb, meshRegistry);
WorldManager wm(geomDb, geomFactory);
Camera cam;
Window win({});
GlSceneRenderer renderer(cam, geomDb, meshRegistry);


int main() {

     // Initial viewport size
    int fbw=0, fbh=0; 
    win.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam.aspect = float(fbw)/float(fbh); 
    }

    // Camera defaults
    cam.eye    = {0.0f, 1.0f, 1.0f};
    cam.up     = {0.0f, 1.0f, 0.0f};
    cam.fovDeg = 60.f;
    cam.yawDeg   = 0.f;
    cam.pitchDeg = -45.f;
    cam.updateVectors();


    // Initial Camera vectors
    GeometryID plane = geomFactory.getPlane();
    GeometryID sphere = geomFactory.getSphere(0.25);

    CreateObjectCommand c1{ plane };
    CreateObjectCommand c2{ sphere };

    wm.apply(PhysicsCommand{c1});
    wm.apply(PhysicsCommand{c2});

    WorldSnapshot s = wm.buildSnapshot();
    assert(s.objects.size() == 2);
    assert(s.objects[0].geom == plane || s.objects[1].geom == plane);

    while (true)
    {
        win.poll();
        if (!win.isOpen()) break;

        double dt = 1.0/60.0; // fixed timestep for now
        wm.step(dt);
        WorldSnapshot snapshot = wm.buildSnapshot();
        renderer.render(snapshot);
    

        win.swap();
    }
    
};
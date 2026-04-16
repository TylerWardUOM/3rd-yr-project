#include "geometry/GeometryFactory.h"
#include "geometry/sdf/PlaneSDF.h"
#include "geometry/sdf/UnitSphereSDF.h"
#include "geometry/sdf/UnitCubeSDF.h"
#include <memory>

// ---- public API ----

GeometryFactory::GeometryFactory(GeometryDatabase& db,
                                 RenderMeshRegistry& meshRegistry)
    : db_(db), meshRegistry_(meshRegistry) {}


GeometryID GeometryFactory::getPlane() {
    if (!planeId_) {
        planeId_ = registerPlane();
    }
    return *planeId_;
}

GeometryID GeometryFactory::getSphere() {
    if (!sphereId_) {
        sphereId_ = registerSphere();
    }
    return *sphereId_;
}

GeometryID GeometryFactory::getCube() { 
    if (!cubeId_) { 
        cubeId_ = registerCube(); 
    } 
    return *cubeId_; 
}

// ---- private helpers ----

GeometryID GeometryFactory::registerPlane() {
    GeometryEntry e;
    e.id = nextId_++;
    e.type = SurfaceType::Plane;

    // Later:
    e.sdf = std::make_shared<PlaneSDF>(Vec3{0,1,0}, 0.0);

    e.renderMesh = meshRegistry_.getOrCreate(MeshKind::Plane);
    // e.physicsShape = ...
    // e.renderMesh = ...

    db_.registerGeometry(e);
    return e.id;
}

GeometryID GeometryFactory::registerSphere() {
    GeometryEntry e;
    e.id = nextId_++;
    e.type = SurfaceType::Sphere;


    e.sdf = std::make_shared<UnitSphereSDF>();
    e.renderMesh = meshRegistry_.getOrCreate(MeshKind::Sphere);



    db_.registerGeometry(e);
    return e.id;
}

GeometryID GeometryFactory::registerCube() { 
    GeometryEntry e; 
    e.id = nextId_++;
    e.type = SurfaceType::Cube;  
    e.sdf = std::make_shared<UnitCubeSDF>(); 
    e.renderMesh = meshRegistry_.getOrCreate(MeshKind::Cube);
    db_.registerGeometry(e); 
    return e.id; 
}
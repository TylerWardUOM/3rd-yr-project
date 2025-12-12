#include "geometry/GeometryFactory.h"
#include "geometry/sdf/PlaneSDF.h"
// #include "geometry/sdf/SphereSDF.h"
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

GeometryID GeometryFactory::getSphere(double radius) {
    auto it = sphereIds_.find(radius);
    if (it != sphereIds_.end()) {
        return it->second;
    }
    GeometryID id = registerSphere(radius);
    sphereIds_[radius] = id;
    return id;
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

GeometryID GeometryFactory::registerSphere(double radius) {
    GeometryEntry e;
    e.id = nextId_++;
    e.type = SurfaceType::Sphere;

    // Later:
    // e.sdf = std::make_shared<SphereSDF>(radius);
    // e.physicsShape = ...
    // e.renderMesh = ...

    db_.registerGeometry(e);
    return e.id;
}

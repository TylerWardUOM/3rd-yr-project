// world/World.cpp
#include "world/World.h"
#include <unordered_map>

World::EntityId World::createEntity() {
    EntityId id = nextId_++;
    entities_.push_back(id);
    return id;
}

World::EntityId World::addPlane(const Pose& T_ws, glm::vec3 colour) {
    auto id = createEntity();
    SurfaceDef s{}; s.id=id; s.type=SurfaceType::Plane; s.T_ws=T_ws; s.colour=colour;
    surfaces_.push_back(s);
    return id;
}
World::EntityId World::addSphere(const Pose& T_ws, double r, glm::vec3 colour) {
    auto id = createEntity();
    SurfaceDef s{}; s.id=id; s.type=SurfaceType::Sphere; s.T_ws=T_ws; s.sphere.radius=r; s.colour=colour;
    surfaces_.push_back(s);
    return id;
}
World::EntityId World::addTriMesh(const Pose& T_ws, MeshId mesh, glm::vec3 colour) {
    auto id = createEntity();
    SurfaceDef s{}; s.id=id; s.type=SurfaceType::TriMesh; s.T_ws=T_ws; s.mesh=mesh; s.colour=colour;
    surfaces_.push_back(s);
    return id;
}

bool World::setPose(EntityId id, const Pose& T_ws) {
    for (auto& s : surfaces_) {
        if (s.id == id) {
            s.T_ws = T_ws;
            return true;
        }
    }
    return false; // no matching entity
}



void World::publishSnapshot(double t_sec) {
    WorldSnapshot snap{};
    snap.t_sec = t_sec;

    // surfaces
    snap.numSurfaces = (uint32_t)std::min<size_t>(surfaces_.size(), std::size(snap.surfaces));
    for (uint32_t i=0; i<snap.numSurfaces; ++i) snap.surfaces[i] = surfaces_[i];

 
    snapBuf_.write(snap);
}

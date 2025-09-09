// world/World.h
#pragma once
#include <vector>
#include <cstdint>
#include "Pose.h"
#include "SurfaceDef.h"
#include "WorldSnapshot.h"
#include "DoubleBuffer.h"

class World {
public:
    using EntityId = uint32_t;

    World() = default;

    // ---- Creation API ----
    EntityId createEntity();

    // New entity with surface
    EntityId addPlane(const Pose& T_ws); // Plane at T_ws
    EntityId addSphere(const Pose& T_ws, double radius); // Sphere with radius
    EntityId addTriMesh(const Pose& T_ws, MeshId meshId); // TriMesh with mesh resource id

    // ---- Authoritative state access (physics writes) ----
    std::vector<SurfaceDef>&       surfaces()       { return surfaces_; }
    const std::vector<SurfaceDef>& surfaces() const { return surfaces_; }


    // ---- Snapshot handoff (physics -> haptics/render) ----
    void publishSnapshot(double t_sec);       // packs surfaces_ into buf
    WorldSnapshot readSnapshot() const { return snapBuf_.read(); }

    bool setPose(EntityId id, const Pose& T_ws);
    bool translate(EntityId id, const glm::dvec3& dp);   // optional convenience
    bool rotate(EntityId id, const glm::dquat& dq);      // optional convenience

private:
    EntityId nextId_ = 1;
    std::vector<EntityId> entities_;
    std::vector<SurfaceDef> surfaces_;

    DoubleBuffer<WorldSnapshot> snapBuf_;
};

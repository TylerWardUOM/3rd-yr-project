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
    EntityId addPlane(const Pose& T_ws, glm::vec3 colour); // Plane at T_ws
    EntityId addSphere(const Pose& T_ws, double radius, glm::vec3 colour); // Sphere with radius
    EntityId addTriMesh(const Pose& T_ws, MeshId meshId, glm::vec3 colour); // TriMesh with mesh resource id

    // ---- Authoritative state access (physics writes) ----
    std::vector<SurfaceDef>&       surfaces()       { return surfaces_; }
    const std::vector<SurfaceDef>& surfaces() const { return surfaces_; }


    // ---- Snapshot handoff (physics -> haptics/render) ----
    void publishSnapshot(double t_sec);       // packs surfaces_ into buf
    WorldSnapshot readSnapshot() const { return snapBuf_.read(); }

    bool setToolPose(const Pose& T_ws);
    Pose readToolPose() const { return toolPoseBuf_.read(); }

    bool setPose(EntityId id, const Pose& T_ws);
    bool setColour(EntityId id, const Colour& colour);
    void setRole(EntityId id, Role r);
    EntityId entityFor(Role r) const;                   // returns 0 if none
    bool translate(EntityId id, const glm::dvec3& dp);   
    bool rotate(EntityId id, const glm::dquat& dq);     

    static int findSurfaceIndexById(const WorldSnapshot& snap, World::EntityId id) {
        for (uint32_t i = 0; i < snap.numSurfaces; ++i) {
            if (snap.surfaces[i].id == id) return (int)i;
        }
        return -1;
    }
private:
    EntityId nextId_ = 1;
    DoubleBuffer<Pose> toolPoseBuf_;
    std::vector<EntityId> entities_;
    std::vector<SurfaceDef> surfaces_;


    DoubleBuffer<WorldSnapshot> snapBuf_;
};
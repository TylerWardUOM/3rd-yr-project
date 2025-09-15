#pragma once
#include "world/World.h"
#include "physics/PhysicsBuffers.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include "physics/PhysicsProps.h"
#include <physx/PxPhysicsAPI.h> // Include the PhysX SDK header

class PhysicsEnginePhysX {
public:
    PhysicsEnginePhysX(World& world, PhysicsBuffers& pbufs);
    ~PhysicsEnginePhysX();

    // Advance physics by dt seconds (accumulates & substeps internally)
    void step(double dt);

    void setPhysicsProps(World::EntityId id, const PhysicsProps& p) { physicsProps_[id] = p; }
    PhysicsProps getPhysicsProps(World::EntityId id) const {
        auto it = physicsProps_.find(id);
        if (it != physicsProps_.end()) return it->second;
        return PhysicsProps(); // default
    }

    void rebuildActors() { buildActorsFromWorld(); } // recreate actors from current World surfaces

private:
    // -- External state
    World&          world_;
    PhysicsBuffers& pbufs_;

    // -- PhysX core
    physx::PxDefaultAllocator      allocator_;
    physx::PxDefaultErrorCallback  errorCb_;
    physx::PxFoundation*           foundation_ = nullptr;
    physx::PxPhysics*              physics_    = nullptr;
    physx::PxPvd*                  pvd_        = nullptr;      // optional
    physx::PxCudaContextManager*   cudaMgr_    = nullptr;      // optional
    physx::PxScene*                scene_      = nullptr;
    physx::PxDefaultCpuDispatcher* dispatcher_ = nullptr;
    physx::PxMaterial*             material_   = nullptr;

    // map World::EntityId -> PhysX actor
    std::unordered_map<World::EntityId, PhysicsProps> physicsProps_;
    std::unordered_map<World::EntityId, physx::PxRigidActor*> actors_;

    // fixed-step accumulator
    double accumulator_   = 0.0;
    double fixedDt_       = 1.0 / 240.0;  // 240 Hz physics

    // --- lifecycle
    void initPhysX();
    void shutdownPhysX();
    void buildActorsFromWorld();  // create actors for current surfaces

    // --- per-step
    void consumeCommands_Once();
    void simulateFixed_(double dt);
    void writeBackPosesToWorld_();

    // --- command helpers
    void applyForceAtPoint(World::EntityId id,
                           const glm::dvec3& F_ws,
                           const glm::dvec3& p_ws,
                           double duration_s);

    // --- type conversions
    static physx::PxVec3 toPx(const glm::dvec3& v) { return physx::PxVec3((float)v.x,(float)v.y,(float)v.z); }
    static physx::PxQuat toPx(const glm::dquat& q) { return physx::PxQuat((float)q.x,(float)q.y,(float)q.z,(float)q.w); }
    static physx::PxTransform toPx(const Pose& T)  { return physx::PxTransform(toPx(T.p), toPx(glm::normalize(T.q))); }

    static glm::dvec3 toGlm(const physx::PxVec3& v){ return glm::dvec3(v.x, v.y, v.z); }
    static glm::dquat toGlm(const physx::PxQuat& q){ return glm::dquat(q.w, q.x, q.y, q.z); } // (w,x,y,z) ctor
    static Pose       toPose(const physx::PxTransform& X){ Pose T; T.p = toGlm(X.p); T.q = glm::normalize(toGlm(X.q)); return T; }

    // utility
    static physx::PxFilterFlags defaultFilterShader(
        physx::PxFilterObjectAttributes, physx::PxFilterData,
        physx::PxFilterObjectAttributes, physx::PxFilterData,
        physx::PxPairFlags&, const void*, physx::PxU32);
};

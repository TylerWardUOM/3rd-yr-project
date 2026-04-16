#pragma once

// Core world/geometry
#include "world/WorldManager.h"
#include "geometry/GeometryDatabase.h"
#include "data/PhysicsProps.h"
#include "data/core/Ids.h"

// Messaging
#include "messaging/Channel.h"
#include "data/Commands.h"          // ToolStateMsg, HapticWrenchCmd (your types)
#include "data/HapticMessages.h"

// Math
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// PhysX
#include <PxPhysicsAPI.h>

// STL
#include <unordered_map>
#include <vector>
#include <optional>

// ------------------------------------------------------------
// PhysicsEnginePhysX
//  - Deterministic fixed-step PhysX simulation
//  - Reads commands (impulses / forces) + optional tool state
//  - Writes poses back into WorldManager (authority remains world+physics)
//  - Does NOT publish WorldSnapshots (WorldManager does that)
// ------------------------------------------------------------
class PhysicsEnginePhysX {
public:

    PhysicsEnginePhysX(
        WorldManager& wm,
        const GeometryDatabase& geomDb,
        msg::Channel<HapticWrenchCmd>& wrenchIn,   // haptics/other -> physics (impulses/forces)
        msg::Channel<ToolStateMsg>&    toolIn,     // optional tool pose/vel (for kinematic tool)
        msg::Channel<HapticWrenchCmd>& wrenchOut   // physics -> haptics (optional raw contact wrench)
    );

    ~PhysicsEnginePhysX();

    // Non-copyable
    PhysicsEnginePhysX(const PhysicsEnginePhysX&)            = delete;
    PhysicsEnginePhysX& operator=(const PhysicsEnginePhysX&) = delete;

    // Main tick: consumes input once, substeps internally at fixedDt_, writes back poses
    void step(double dt);

    // Rebuild actors from current world state/topology (call when topology/props changed)
    void rebuildActors();


    // Fixed-step control
    void   setFixedDt(double fixedDt) { fixedDt_ = fixedDt; }
    double fixedDt() const            { return fixedDt_; }

private:
    // ------------------------------------------------------------
    // External (authoritative) state
    // ------------------------------------------------------------
    WorldManager&            wm_;
    const GeometryDatabase&  geomDb_;

    msg::Channel<HapticWrenchCmd>& wrenchIn_;
    msg::Channel<ToolStateMsg>&    toolIn_;
    msg::Channel<HapticWrenchCmd>& wrenchOut_;

    // ------------------------------------------------------------
    // PhysX core
    // ------------------------------------------------------------
    physx::PxDefaultAllocator      allocator_;
    physx::PxDefaultErrorCallback  errorCb_;
    physx::PxFoundation*           foundation_ = nullptr;
    physx::PxPhysics*              physics_    = nullptr;
    physx::PxPvd*                  pvd_        = nullptr;   // optional
    physx::PxScene*                scene_      = nullptr;
    physx::PxDefaultCpuDispatcher* dispatcher_ = nullptr;

    // Default material + optional per-entity override materials
    physx::PxMaterial* materialDefault_ = nullptr;
    std::unordered_map<ObjectID, physx::PxMaterial*> materials_; // owned; released on shutdown

    // Map entity -> PhysX actor
    std::unordered_map<ObjectID, physx::PxRigidActor*> actors_;   // owned by scene; released on shutdown

    // Fixed-step accumulator
    double accumulator_ = 0.0;
    double fixedDt_     = 1.0 / 240.0; // 240 Hz

private:
    // ------------------------------------------------------------
    // Lifecycle
    // ------------------------------------------------------------
    void initPhysX_();
    void shutdownPhysX_();

    void clearActors_();
    void clearMaterials_();

    void buildActorsFromWorld_();     // uses WorldManager objects/surfaces
    physx::PxMaterial* materialFor_(ObjectID id, const PhysicsProps& p);

    // ------------------------------------------------------------
    // Per-step pipeline
    // ------------------------------------------------------------
    void consumeInputsOnce_();        // drain wrenchIn_ (and tool state if needed)
    void simulateFixed_(double dt);   // accumulator + substeps
    void writeBackPoses_();           // dynamic actors -> wm_.setPose(...)

    // ------------------------------------------------------------
    // Command helpers
    // ------------------------------------------------------------
    void applyImpulseAtPoint_(
        ObjectID id,
        const glm::dvec3& J_ws,       // impulse (N*s)
        const glm::dvec3& p_ws        // world point
    );

    // ------------------------------------------------------------
    // Conversions
    // ------------------------------------------------------------
    static physx::PxVec3 toPx(const glm::dvec3& v) {
        return physx::PxVec3((float)v.x, (float)v.y, (float)v.z);
    }

    static physx::PxQuat toPx(const glm::dquat& q) {
        return physx::PxQuat((float)q.x, (float)q.y, (float)q.z, (float)q.w);
    }

    static glm::dvec3 toGlm(const physx::PxVec3& v) {
        return glm::dvec3(v.x, v.y, v.z);
    }

    static glm::dquat toGlm(const physx::PxQuat& q) {
        return glm::dquat(q.w, q.x, q.y, q.z); // (w,x,y,z)
    }

    static Pose toPose(const physx::PxTransform& X) {
        Pose T;
        T.p = toGlm(X.p);
        T.q = glm::normalize(toGlm(X.q));
        return T;
    }

    static physx::PxTransform toPx(const Pose& T) {
        return physx::PxTransform(toPx(T.p), toPx(glm::normalize(T.q)));
    }

    // ------------------------------------------------------------
    // Collision filtering
    // ------------------------------------------------------------
    static physx::PxFilterFlags defaultFilterShader_(
        physx::PxFilterObjectAttributes, physx::PxFilterData,
        physx::PxFilterObjectAttributes, physx::PxFilterData,
        physx::PxPairFlags&, const void*, physx::PxU32
    );
};

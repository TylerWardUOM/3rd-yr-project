// PhysXEngine.cpp
#include "engines/PhysicsEnginePhysX.h"

#include <PxRigidBody.h>
#include <extensions/PxRigidActorExt.h>
#include <extensions/PxRigidBodyExt.h>

#include <iostream>
#include <chrono>

using namespace physx;

// ------------------------------------------------------------
// Ctor / Dtor
// ------------------------------------------------------------
PhysicsEnginePhysX::PhysicsEnginePhysX(
    WorldManager& wm,
    const GeometryDatabase& geomDb,
    msg::Channel<HapticWrenchCmd>& wrenchIn,
    msg::Channel<ToolStateMsg>& toolIn,
    msg::Channel<HapticWrenchCmd>& wrenchOut
)
    : wm_(wm)
    , geomDb_(geomDb)
    , wrenchIn_(wrenchIn)
    , toolIn_(toolIn)
    , wrenchOut_(wrenchOut)
{
    initPhysX_();
    buildActorsFromWorld_();
}

PhysicsEnginePhysX::~PhysicsEnginePhysX() {
    shutdownPhysX_();
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
void PhysicsEnginePhysX::step(double dt) {
    // 1) If topology/props changed, rebuild actors
    if (wm_.consumeDirty(WorldDirty::Topology | WorldDirty::Physics)) {
        rebuildActors();
    }


    // 2) Consume inputs once per external tick
    consumeInputsOnce_();

    // 3) Fixed-step simulate
    simulateFixed_(dt);

    // 4) Write-back updated poses into WorldManager
    writeBackPoses_();
}

void PhysicsEnginePhysX::rebuildActors() {
    buildActorsFromWorld_();
}


// ------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------
void PhysicsEnginePhysX::initPhysX_() {
    foundation_ = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_, errorCb_);
    if (!foundation_) {
        std::cerr << "PhysX: failed to create foundation\n";
        return;
    }

    PxTolerancesScale scale; // defaults (meters, seconds)
    physics_ = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_, scale, true, pvd_);
    if (!physics_) {
        std::cerr << "PhysX: failed to create physics\n";
        return;
    }

    PxSceneDesc desc(physics_->getTolerancesScale());
    desc.gravity = PxVec3(0.f, -9.81f, 0.f); // set to zero if you want no gravity
    dispatcher_ = PxDefaultCpuDispatcherCreate(2);
    desc.cpuDispatcher = dispatcher_;
    desc.filterShader  = &PhysicsEnginePhysX::defaultFilterShader_;
    desc.flags |= PxSceneFlag::eENABLE_CCD;

    scene_ = physics_->createScene(desc);
    if (!scene_) {
        std::cerr << "PhysX: failed to create scene\n";
        return;
    }

    // Default material (used unless you want per-entity materials)
    materialDefault_ = physics_->createMaterial(0.6f, 0.6f, 0.1f);
}

void PhysicsEnginePhysX::shutdownPhysX_() {
    clearActors_();
    clearMaterials_();

    if (materialDefault_) { materialDefault_->release(); materialDefault_ = nullptr; }
    if (scene_)           { scene_->release();           scene_ = nullptr; }
    if (dispatcher_)      { dispatcher_->release();      dispatcher_ = nullptr; }
    if (physics_)         { physics_->release();         physics_ = nullptr; }
    if (pvd_)             { pvd_->release();             pvd_ = nullptr; }
    if (foundation_)      { foundation_->release();      foundation_ = nullptr; }
}

void PhysicsEnginePhysX::clearActors_() {
    if (!scene_) { actors_.clear(); return; }
    for (auto& kv : actors_) {
        if (kv.second) {
            scene_->removeActor(*kv.second);
            kv.second->release();
        }
    }
    actors_.clear();
}

void PhysicsEnginePhysX::clearMaterials_() {
    for (auto& kv : materials_) {
        if (kv.second) kv.second->release();
    }
    materials_.clear();
}

PxMaterial* PhysicsEnginePhysX::materialFor_(ObjectID id, const PhysicsProps& p) {
    // If you want per-entity materials, create & own them here.
    // Otherwise always return materialDefault_.
    //
    // Per-entity is useful if you need different friction/restitution per object.
    // It costs memory but is fine for your project scale.

    auto it = materials_.find(id);
    if (it != materials_.end()) return it->second;

    PxMaterial* mat = physics_->createMaterial(
        (PxReal)p.staticFriction,
        (PxReal)p.dynamicFriction,
        (PxReal)p.restitution
    );
    materials_[id] = mat;
    return mat;
}

// ------------------------------------------------------------
// Build actors from authoritative world state
// ------------------------------------------------------------
void PhysicsEnginePhysX::buildActorsFromWorld_() {
    if (!physics_ || !scene_) return;

    // Rebuild = clear old, then recreate from current world snapshot
    clearActors_();
    clearMaterials_();

    // We use a snapshot as the “read-only view” of the authoritative world.
    // If you later expose a direct iterator over objects, you can use that instead.
    WorldSnapshot snap = wm_.buildSnapshot();

    for (const auto& obj : snap.objects) {
        const ObjectID id = obj.id;
        const PhysicsProps p = wm_.getPhysicsProps(id);

        const PxTransform X = toPx(obj.T_ws);

        // --- Geometry lookup ---
        const GeometryEntry& ge = geomDb_.get(obj.geom);

        PxRigidActor* actor = nullptr;

        if (ge.type == SurfaceType::Plane) {
            // Planes must be static; use a thin box “ground plane”
            auto* a = physics_->createRigidStatic(X);
            PxBoxGeometry geom(PxReal(1000), PxReal(0.01), PxReal(1000));
            PxMaterial* mat = materialFor_(id, p);
            PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, geom, *mat);

            sh->setContactOffset(0.02f);
            sh->setRestOffset(0.0f);
            sh->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
            sh->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

            scene_->addActor(*a);
            actor = a;
        }
        else if (ge.type == SurfaceType::Sphere) {
            // Replace radius with geomDb param:
            // double r = ge.sphere.radius;
            double r = obj.T_ws.s; // use uniform scale as radius

            PxSphereGeometry geom((PxReal)r);
            PxMaterial* mat = materialFor_(id, p);

            if (p.dynamic) {
                auto* a = physics_->createRigidDynamic(X);
                PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, geom, *mat);

                a->setLinearDamping((PxReal)p.linDamping);
                a->setAngularDamping((PxReal)p.angDamping);

                if (p.kinematic) {
                    a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                } else {
                    a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
                }

                // Mass/inertia
                if (p.mass.has_value()) {
                    PxRigidBodyExt::setMassAndUpdateInertia(*a, (PxReal)(*p.mass));
                } else {
                    PxRigidBodyExt::updateMassAndInertia(*a, (PxReal)p.density);
                }

                // Contact tuning
                sh->setContactOffset(0.02f);
                sh->setRestOffset(0.0f);

                // CCD + solver
                a->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
                a->setSolverIterationCounts(/*posIters=*/8, /*velIters=*/2);

                scene_->addActor(*a);
                actor = a;
            } else {
                auto* a = physics_->createRigidStatic(X);
                PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, geom, *mat);
                sh->setContactOffset(0.02f);
                sh->setRestOffset(0.0f);
                scene_->addActor(*a);
                actor = a;
            }
        }

        else if (ge.type == SurfaceType::Cube){
            const PxReal half = PxReal(0.5 * obj.T_ws.s);   // half-extents
            PxBoxGeometry boxGeom(half, half, half);
            PxMaterial* mat = materialFor_(id, p);

            if (p.dynamic) {
                auto* a = physics_->createRigidDynamic(X);
                PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, boxGeom, *mat);

                a->setLinearDamping((PxReal)p.linDamping);
                a->setAngularDamping((PxReal)p.angDamping);

                if (p.kinematic) {
                    a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                } else {
                    a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
                }

                // Mass/inertia
                if (p.mass.has_value()) {
                    PxRigidBodyExt::setMassAndUpdateInertia(*a, (PxReal)(*p.mass));
                } else {
                    PxRigidBodyExt::updateMassAndInertia(*a, (PxReal)p.density);
                }

                // Contact tuning
                sh->setContactOffset(0.02f);
                sh->setRestOffset(0.0f);

                // CCD + solver
                a->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
                a->setSolverIterationCounts(/*posIters=*/8, /*velIters=*/2);

                scene_->addActor(*a);
                actor = a;
            } else {
                auto* a = physics_->createRigidStatic(X);
                PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, boxGeom, *mat);
                sh->setContactOffset(0.02f);
                sh->setRestOffset(0.0f);
                scene_->addActor(*a);
                actor = a;
            }
        }
        // TODO: TriMesh -> cook/create PxTriangleMeshGeometry and create static/ dynamic as needed.

        if (actor) {
            actors_[id] = actor;
        }
    }
}

// ------------------------------------------------------------
// Per-step pipeline
// ------------------------------------------------------------
void PhysicsEnginePhysX::consumeInputsOnce_() {
    // 1) Drain incoming wrench/impulse commands
    std::vector<HapticWrenchCmd> cmds;
    wrenchIn_.drain(cmds);

    for (const auto& c : cmds) {
        // You previously treated this as "impulse = F * duration".
        // Keep that contract unless you switch to explicit impulse messages.
        const glm::dvec3 J_ws = c.force_ws * c.duration_s;
        applyImpulseAtPoint_(c.targetId, J_ws, c.point_ws);
    }

    // 2) Optional tool state consumption (store latest if you need it)
    // ToolStateMsg tool;
    // while (toolIn_.try_pop(tool)) { latestTool_ = tool; }
    // If you use a kinematic tool actor, update it here before sim.
}

void PhysicsEnginePhysX::simulateFixed_(double dt) {
    accumulator_ += dt;

    while (accumulator_ >= fixedDt_) {
        scene_->simulate((PxReal)fixedDt_);
        scene_->fetchResults(true);
        accumulator_ -= fixedDt_;
    }
}

void PhysicsEnginePhysX::writeBackPoses_() {
    // Write back only dynamic actors (statics don't move)
    for (auto& kv : actors_) {
        const ObjectID id = kv.first;
        PxRigidActor* a = kv.second;
        if (!a) continue;

        if (auto* dyn = a->is<PxRigidDynamic>()) {
            // Skip kinematic bodies if you don't want PhysX to overwrite your pose
            // if (dyn->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) {
            //     continue;
            // }

            const PxTransform X = dyn->getGlobalPose();
            const Pose T = toPose(X);

            // WorldManager is authoritative; physics writes into it.
            //LATER CHANGE SO I CAN HAVE MODULAR PHSYICS
            wm_.setPose(id, T);
        }
    }
}

// ------------------------------------------------------------
// Command helper
// ------------------------------------------------------------
void PhysicsEnginePhysX::applyImpulseAtPoint_(
    ObjectID id,
    const glm::dvec3& J_ws,
    const glm::dvec3& p_ws
) {
    auto it = actors_.find(id);
    if (it == actors_.end()) return;

    if (auto* body = it->second->is<PxRigidBody>()) {
        PxRigidBodyExt::addForceAtPos(
            *body,
            toPx(J_ws),   // impulse (N*s)
            toPx(p_ws),   // world point
            PxForceMode::eIMPULSE,
            true          // autowake
        );
    }
}

// ------------------------------------------------------------
// Filter shader
// ------------------------------------------------------------
PxFilterFlags PhysicsEnginePhysX::defaultFilterShader_(
    PxFilterObjectAttributes, PxFilterData,
    PxFilterObjectAttributes, PxFilterData,
    PxPairFlags& pairFlags,
    const void*, PxU32
) {
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    return PxFilterFlag::eDEFAULT;
}

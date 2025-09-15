#include "physics/PhysXEngine.h"
#include "physics/PhysicsCommands.h"
#include <physx/PxRigidBody.h> // PxRigidBodyExt::addForceAtPos
#include <physx/extensions/PxRigidActorExt.h>
#include <physx/extensions/PxRigidBodyExt.h> // PxRigidBodyExt::setMassAndUpdateInertia
#include <chrono>
#include <iostream>

using namespace physx;

// ---------- ctor / dtor ----------
PhysicsEnginePhysX::PhysicsEnginePhysX(World& world, PhysicsBuffers& pbufs)
    : world_(world), pbufs_(pbufs) {
    initPhysX();
    buildActorsFromWorld();
}

PhysicsEnginePhysX::~PhysicsEnginePhysX() {
    shutdownPhysX();
}

// ---------- init / shutdown ----------
void PhysicsEnginePhysX::initPhysX() {
    foundation_ = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_, errorCb_);

    PxTolerancesScale scale; // default meters/seconds
    physics_ = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_, scale, true, pvd_);

    PxSceneDesc desc(physics_->getTolerancesScale());
    desc.gravity = PxVec3(0.f, -9.81f, 0.f); // no gravity for now
    dispatcher_  = PxDefaultCpuDispatcherCreate(2);
    desc.cpuDispatcher = dispatcher_;
    desc.filterShader  = &PhysicsEnginePhysX::defaultFilterShader;
    desc.flags      |= PxSceneFlag::eENABLE_CCD; // enable CCD globally

    scene_ = physics_->createScene(desc);
    material_ = physics_->createMaterial(0.6f, 0.6f, 0.1f); // staticFric, dynamicFric, restitution
}

void PhysicsEnginePhysX::shutdownPhysX() {
    // release actors
    if (scene_) {
        for (auto& kv : actors_) {
            if (kv.second) { scene_->removeActor(*kv.second); kv.second->release(); }
        }
        actors_.clear();
    }
    if (material_)  { material_->release();  material_ = nullptr; }
    if (scene_)     { scene_->release();     scene_ = nullptr; }
    if (dispatcher_){ dispatcher_->release();dispatcher_ = nullptr; }
    if (physics_)   { physics_->release();   physics_ = nullptr; }
    if (pvd_)       { pvd_->release();       pvd_ = nullptr; }
    if (foundation_){ foundation_->release();foundation_ = nullptr; }
}

// ---------- build actors from World ----------
void PhysicsEnginePhysX::buildActorsFromWorld() {
    // Clear any previous actors 
    for (auto& kv : actors_) {
        if (kv.second) { scene_->removeActor(*kv.second); kv.second->release(); }
    }
    actors_.clear();

    for (const auto& s : world_.surfaces()) {
        // 1) Fetch physics props (or defaults)
        const PhysicsProps* pp = getPhysicsProps(s.id);
        PhysicsProps p = pp ? *pp : PhysicsProps{};  // dynamic=true, etc.

        const PxTransform X = toPx(s.T_ws); // world pose

        // 2) Per-entity material 
        PxMaterial* mat = physics_->createMaterial(
            p.staticFriction, p.dynamicFriction, p.restitution);

        PxRigidActor* actor = nullptr; // created actor

        switch (s.type) {
            case SurfaceType::Plane: {
                // Planes must be static in PhysX
                auto* a = physics_->createRigidStatic(X);
                PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, PxPlaneGeometry(), *mat); // infinite plane
                sh->setContactOffset(0.02f);
                sh->setRestOffset(0.0f);
                sh->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
                sh->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
                scene_->addActor(*a);
                actor = a;
                break;
            }

            case SurfaceType::Sphere: {
                const PxSphereGeometry geom((float)s.sphere.radius);
                if (p.dynamic) {
                    auto* a = physics_->createRigidDynamic(X);
                    PxShape* sh = PxRigidActorExt::createExclusiveShape(*a, geom, *mat);
                    a->setLinearDamping(p.linDamping);
                    a->setAngularDamping(p.angDamping);
                    if (p.kinematic) {
                        a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    }
                    if (p.mass.has_value()) {
                        PxRigidBodyExt::setMassAndUpdateInertia(*a, *p.mass);
                    } else {
                        PxRigidBodyExt::updateMassAndInertia(*a, p.density);
                    }
                    sh->setContactOffset(0.02f);
                    sh->setRestOffset(0.0f);
                    a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);      // ensure dynamic
                    a->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);    // per-body CCD
                    a->setSolverIterationCounts(/*posIters=*/8, /*velIters=*/2); // stronger solver
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
                break;
            }

            case SurfaceType::TriMesh: {
                // TODO: cook and create PxTriangleMeshGeometry.
                // For now skip or create a static placeholder if you have cooked data.
                // continue; // if skipping
                break;
            }
        }

        if (actor) {
            actors_[s.id] = actor;
        }
        mat->release();
    }
}


// ---------- step ----------
void PhysicsEnginePhysX::step(double dt) {
    // 1) consume commands once per external tick
    consumeCommands_Once();

    // 2) fixed-step simulate
    simulateFixed_(dt);

    // 3) write-back updated poses into World and publish
    writeBackPosesToWorld_();
    world_.publishSnapshot(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

void PhysicsEnginePhysX::consumeCommands_Once() {
    PhysicsCommands cmds = pbufs_.cmdBuf.read(); // whatever haptics wrote last

    for (const auto& c : cmds.wrenches) {
        applyForceAtPoint(c.body, c.F_ws, c.p_ws, c.duration_s);
    }
    // IMPORTANT: do NOT write back here (we consume-and-forget)
}

void PhysicsEnginePhysX::simulateFixed_(double dt) {
    accumulator_ += dt;
    while (accumulator_ >= fixedDt_) {
        scene_->simulate((PxReal)fixedDt_);
        scene_->fetchResults(true);
        accumulator_ -= fixedDt_;
    }
}

void PhysicsEnginePhysX::writeBackPosesToWorld_() {
    // Iterate only dynamic actors we expect to move (spheres in this simple build)
    for (auto& kv : actors_) {
        auto id = kv.first;
        auto* a = kv.second;

        if (auto* dyn = a->is<PxRigidDynamic>()) {
            const PxTransform X = dyn->getGlobalPose();
            Pose T = toPose(X);

            // Update World’s authoritative transform for this entity
            world_.setPose(id, T);
        }
        // statics (planes) do not change pose
    }
}

// ---------- command helper ----------
void PhysicsEnginePhysX::applyForceAtPoint(World::EntityId id,
                                           const glm::dvec3& F_ws,
                                           const glm::dvec3& p_ws,
                                           double /*duration_s*/)
{
    auto it = actors_.find(id);
    if (it == actors_.end()) return;
    if (auto* body = it->second->is<PxRigidBody>()) {
        // This extension computes the equivalent torque and applies both
        PxRigidBodyExt::addForceAtPos(
            *body,
            toPx(F_ws),
            toPx(p_ws),
            PxForceMode::eFORCE,
            true  // autowake
        );
        std::cout << "Applied force " << F_ws.x << ", " << F_ws.y << ", " << F_ws.z << " to entity " << id << std::endl;
    }
}

// ---------- filter shader ----------
PxFilterFlags PhysicsEnginePhysX::defaultFilterShader(
    PxFilterObjectAttributes, PxFilterData,
    PxFilterObjectAttributes, PxFilterData,
    PxPairFlags& pairFlags, const void*, PxU32)
{
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    return PxFilterFlag::eDEFAULT;
}

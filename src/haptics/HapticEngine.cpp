#include "haptics/HapticEngine.h"
#include <thread>
#include "env/primitives/PlaneEnv.h"
#include <stdexcept>

HapticEngine::HapticEngine(World& world)
    : world_(world)
{
    // Initialize tool pose to identity
    toolPose_.write(Pose{});
    proxyPose_.write(Pose{});
    refPose_.write(Pose{});
}

HapticEngine::~HapticEngine() = default;

void HapticEngine::run() {
    // Main haptics loop
    const float dt = 0.001f; // 1ms timestep
    while (true) {
        update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void HapticEngine::update(float dt) {
    // Read current tool pose
    Pose toolPose = toolPose_.read();

    // For simplicity, assume zero velocity and button not pressed
    //glm::dvec3 toolVel{0.0, 0.0, 0.0};

    // Compute forces and update proxy/ref poses
    Pose proxyPose;
    Pose refPose = toolPose;
    glm::dvec3 force;
    WorldSnapshot snap = world_.readSnapshot(); // ensure we have latest world state
    double d;
    for (SurfaceDef const& surf : snap.surfaces) {
        Pose surfPose = surf.T_ws;
        EnvInterface* env = nullptr;
        switch (surf.type)
        {
        case (SurfaceType::Plane):
            glm::dvec3 n_local{ 0.0, 1.0, 0.0 };
            // rotate into world
            glm::dmat3 R = glm::mat3_cast(glm::quat(surfPose.q));   // rotation matrix
            glm::dvec3 n_world = glm::normalize(R * n_local);

            // pick a world point on the plane (pose origin in this convention)
            glm::dvec3 p_world = surfPose.p;

            // plane equation: n�x = d
            d = glm::dot(n_world, p_world);

            // now construct your plane SDF
            env = new PlaneEnv(n_world, d);   
            if (!env) {
                throw std::runtime_error("Failed to allocate PlaneEnv");
            }
            break;
        
        default:
            break;
        }
        if (!env) continue; // unsupported type
        if (env->phi(refPose.p) < 0.0) {
            proxyPose.p = env->project(refPose.p);
            world_.setPose(3, proxyPose);
            world_.publishSnapshot(0.0); // publish updated state
        }
        else {;
            if (refPose.p != proxyPose.p) {
                world_.setPose(3, refPose);
                world_.publishSnapshot(0.0); // publish updated sta                
            }
        }         
        // Future: handle Sphere and TriMesh types
    }
    //computeForces(toolPose, toolVel, toolButton, proxyPose, force, refPose);

    // Write updated proxy/ref poses
    proxyPose_.write(proxyPose);
    refPose_.write(refPose);

}
#include "haptics/HapticEngine.h"
#include <thread>
#include "env/primitives/PlaneEnv.h"
#include "env/primitives/SphereEnv.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <memory>
#include <iostream>

HapticEngine::HapticEngine(World& world, PhysicsBuffers& phys)
    : world_(world), physBufs_(&phys){}

HapticEngine::~HapticEngine() = default;

void HapticEngine::run() {
    // Main haptics loop
    const float dt = 0.005f; // 1ms timestep
    while (true) {
        update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
static std::unique_ptr<EnvInterface> makePlaneEnv(Pose T_ws) {

                    // world-space plane parameters
    glm::dvec3 n_local{0.0, 1.0, 0.0};
    glm::dmat3 R = glm::mat3_cast(T_ws.q);
    glm::dvec3 n_world = glm::normalize(R * n_local);
    //std::cout << "Plane position: " << T_ws.p.x << ", " << T_ws.p.y << ", " << T_ws.p.z << std::endl;
    double d = glm::dot(n_world, T_ws.p);     // plane: phi(x)=n·x - d
    return std::make_unique<PlaneEnv>(n_world, d); // pass WORLD normal, not local
}

static std::unique_ptr<EnvInterface> makeSphereEnv(Pose T_ws, double radius) {
    glm::vec3 center = T_ws.p;
    return std::make_unique<SphereEnv>(center, radius/2); 

}

void HapticEngine::update(float dt) {
    // 1) Inputs
    WorldSnapshot snap = world_.readSnapshot();
    ToolIn toolIn = bufs_.inBuf.read();

    Pose toolPose = toolIn.devicePose_ws;
    //std::cout << "Tool position: " << toolPose.p.x << ", " << toolPose.p.y << ", " << toolPose.p.z << std::endl;
    Pose refPose  = toolPose;                  // simple tracking for now
    Pose proxyPose = proxyPosePrev_;           // start from last

    // Contact search (track best penetration)
    double bestPhi = +1e30;
    World::EntityId contactEntity = 0;
    glm::dvec3 contactPoint_ws{0}, contactNormal_ws{0,1,0};


    // 2) Env collision / projection
    //std::cout << snap.t_sec << std::endl;
    for (const SurfaceDef& surf : snap.surfaces) {
        std::unique_ptr<EnvInterface> env;
        switch (surf.type){
            case (SurfaceType::Plane):
                env = makePlaneEnv(surf.T_ws);        
                if (!env) {
                    throw std::runtime_error("Failed to allocate PlaneEnv");
                }
                break;
            case (SurfaceType::Sphere):
                env = makeSphereEnv(surf.T_ws, surf.sphere.radius);
                if (!env) {
                    throw std::runtime_error("Failed to allocate SphereEnv");
                }
                //std::cout <<"Sphere Position" << surf.T_ws.p.x << ", " << surf.T_ws.p.y << ", " << surf.T_ws.p.z << std::endl;
                break;
            default:
                break;
        }
        if (env){
            const double phi = env->phi(refPose.p); 
            if (phi < bestPhi) {
                bestPhi = phi;
                if (phi < 0.0){
                    contactEntity = surf.id;
                    contactPoint_ws = env->project(refPose.p);
                    contactNormal_ws = env->grad(contactPoint_ws);
                    if (glm::dot(contactNormal_ws, contactNormal_ws) > 0) contactNormal_ws = glm::normalize(contactNormal_ws);
                }
            }
        }
    }

    // Proxy position
    if (bestPhi < 0.0) {
        // constrain proxy to surface
        proxyPose.p = contactPoint_ws;
    } else {
        proxyPose.p = refPose.p;
        contactEntity = 0;
    }


    // Virtual Coupling
    const double K_track = 2000.0; // [N/m]
    const double D_track = 0.7 * 2.0 * std::sqrt(K_track * 0.2); // critical ish damping

    const glm::dvec3 proxyVel = (proxyPose.p - proxyPosePrev_.p) * (1.0 / dt);
    const glm::dvec3 toolVel  = glm::dvec3(0); // assume static tool for now


    glm::dvec3 F_env_on_tool = -K_track * (toolPose.p - proxyPose.p)
                               - D_track * (toolVel - proxyVel);
    // Clamp for safety
    const double Fmax = 15.0; // N
    const double fmag = glm::length(F_env_on_tool);
    if (fmag > Fmax) F_env_on_tool *= (Fmax / fmag);


    // output force to world frame#
    PhysicsCommands cmds{};
    if (physBufs_ && contactEntity != 0) {
        ApplyWrenchAtPoint w{};
        w.body       = contactEntity;
        w.F_ws       = -F_env_on_tool;   // environment gets opposite of what tool feels
        w.p_ws       = contactPoint_ws;  // apply at contact point
        w.duration_s = dt;
        w.t_sec      = 0.0;
        cmds.wrenches.push_back(w);
    }
    physBufs_->cmdBuf.write(cmds);  

    // Outputs for viz/device
    ToolOut toolOut{};
    toolOut.proxyPose_ws = proxyPose;

    HapticSnapshot hs{};
    hs.devicePose_ws = toolPose;
    hs.proxyPose_ws  = proxyPose;
    hs.force_ws      = F_env_on_tool;   // for HUD plotting

    bufs_.outBuf.write(toolOut);
    bufs_.snapBuf.write(hs);

    proxyPosePrev_ = proxyPose;
}
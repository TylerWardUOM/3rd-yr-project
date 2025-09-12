#include "haptics/HapticEngine.h"
#include <thread>
#include "env/primitives/PlaneEnv.h"
#include "env/primitives/SphereEnv.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>

HapticEngine::HapticEngine(World& world)
    : world_(world){}

HapticEngine::~HapticEngine() = default;

void HapticEngine::run() {
    // Main haptics loop
    const float dt = 0.005f; // 1ms timestep
    float time = 0.0f;
    while (true) {
        update(time);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        time += dt;
    }
}
PlaneEnv* planeHelper(Pose T_ws) {

                    // world-space plane parameters
    glm::dvec3 n_local{0.0, 1.0, 0.0};
    glm::dmat3 R = glm::mat3_cast(T_ws.q);
    glm::dvec3 n_world = glm::normalize(R * n_local);
    //std::cout << "Plane position: " << T_ws.p.x << ", " << T_ws.p.y << ", " << T_ws.p.z << std::endl;
    double d = glm::dot(n_world, T_ws.p);     // plane: phi(x)=n·x - d
    PlaneEnv plane(n_local, d);
    return &plane;
}

SphereEnv* sphereHelper(Pose T_ws, double radius) {
    glm::vec3 center = T_ws.p;
    SphereEnv* sphere = new SphereEnv(center, radius/2);
    return sphere;
}

void HapticEngine::update(float time) {
    // 1) Inputs
    WorldSnapshot snap = world_.readSnapshot();
    ToolIn toolIn = bufs_.inBuf.read();
    bool flag = false;
    ToolOut toolOut{};
    HapticSnapshot snapBuf{};
    
    Pose toolPose = toolIn.devicePose_ws;
    //std::cout << "Tool position: " << toolPose.p.x << ", " << toolPose.p.y << ", " << toolPose.p.z << std::endl;
    Pose refPose  = toolPose;                  // simple tracking for now
    Pose proxyPose = proxyPosePrev_;           // start from last
    int selectedIndx = -1;
    // 2) Env collision / projection
    //std::cout << snap.t_sec << std::endl;
    for (const SurfaceDef& surf : snap.surfaces) {
        EnvInterface* env = nullptr;
        switch (surf.type){
            case (SurfaceType::Plane):
                env = planeHelper(surf.T_ws);        
                if (!env) {
                    throw std::runtime_error("Failed to allocate PlaneEnv");
                }
                break;
            case (SurfaceType::Sphere):
                env = sphereHelper(surf.T_ws, surf.sphere.radius);
                if (!env) {
                    throw std::runtime_error("Failed to allocate SphereEnv");
                }
                //std::cout <<"Sphere Position" << surf.T_ws.p.x << ", " << surf.T_ws.p.y << ", " << surf.T_ws.p.z << std::endl;
                break;
            default:
                break;
        }
        if (env){
            if (env->phi(refPose.p) < 0.0) {
                proxyPose.p = env->project(refPose.p);
                flag=true;
            } else {
                if (!flag){
                    proxyPose.p = refPose.p;
                }
            }
        }
    }
    //world_.setPose(toolId_, toolPose); // update t for debug
    snapBuf.devicePose_ws = toolPose;
    // 3) Write outputs to world entities (by role)
    toolOut.proxyPose_ws = proxyPose; // for viz
    snapBuf.proxyPose_ws = proxyPose;
        //std::cout << "Proxy position: " << proxyPose.p.x << ", " << proxyPose.p.y << ", " << proxyPose.p.z << std::endl;


    // 4) Snapshot once per tick
    bufs_.outBuf.write(toolOut);
    bufs_.snapBuf.write(snapBuf);

    // 5) Keep state
    proxyPosePrev_ = proxyPose;
}
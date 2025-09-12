#pragma once
#include "haptics/HapticBuffers.h"
#include "physics/PhysicsBuffers.h"
#include "world/World.h"

class HapticEngine {

    public:
        HapticEngine(World& world, PhysicsBuffers& phys);
        ~HapticEngine();

        void run(); // main haptics loop
        
        HapticSnapshot readSnapshot() const { return bufs_.snapBuf.read(); }

        void submitToolPose(const Pose& T_ws, double t_sec) {
            ToolIn in = bufs_.inBuf.read();
            in.devicePose_ws = T_ws;
            in.t_sec = t_sec;
            bufs_.inBuf.write(in);
        }


    private:
        World& world_; // shared state with scene/physics thread

        HapticsBuffers bufs_; // local buffers
        Pose proxyPosePrev_{}; // previous proxy pose for velocity calc
        PhysicsBuffers* physBufs_ = nullptr;


        // --- core loop functions ---
        void update(float dt); // update haptics state
        void computeForces(const Pose& toolPose, const glm::dvec3& toolVel, bool toolButton,
                           Pose& proxyPose, glm::dvec3& force, Pose& refPose);
}; 
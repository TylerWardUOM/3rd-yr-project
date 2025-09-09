#pragma once
#include "world/World.h"

class HapticEngine {

    public:
        HapticEngine(World& world);
        ~HapticEngine();

        void run(); // main haptics loop
        
        void getProxyPose(Pose& p) const { p = proxyPose_.read(); }
        void getRefPose(Pose& p) const { p = refPose_.read(); }

        void setToolPose(const Pose& p) { toolPose_.write(p); }


    private:
        World& world_; // shared state with scene/physics thread

        // tool state (from scene/device)
        DoubleBuffer<Pose>      toolPose_{};

        // proxy/ref state (to scene)
        DoubleBuffer<Pose>      proxyPose_{};
        DoubleBuffer<Pose>      refPose_{};

        // --- core loop functions ---
        void update(float dt); // update haptics state
        void computeForces(const Pose& toolPose, const glm::dvec3& toolVel, bool toolButton,
                           Pose& proxyPose, glm::dvec3& force, Pose& refPose);
}; 
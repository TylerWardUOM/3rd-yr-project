#pragma once
#include "world/World.h"

class HapticEngine {

    public:
        HapticEngine(World& world);
        ~HapticEngine();

        void run(); // main haptics loop

        void bindRoles(){
            toolId_ = world_.entityFor(Role::Tool);
            proxyId_ = world_.entityFor(Role::Proxy);
            refId_ = world_.entityFor(Role::Reference);
        }
        


    private:
        World& world_; // shared state with scene/physics thread

        World::EntityId toolId_{0}, proxyId_{0}, refId_{0};

        Pose proxyPosePrev_{}; // previous proxy pose for velocity calc


        // --- core loop functions ---
        void update(float dt); // update haptics state
        void computeForces(const Pose& toolPose, const glm::dvec3& toolVel, bool toolButton,
                           Pose& proxyPose, glm::dvec3& force, Pose& refPose);
}; 
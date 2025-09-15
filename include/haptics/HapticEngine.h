#pragma once
#include "haptics/HapticBuffers.h"
#include "physics/PhysicsBuffers.h"
#include "world/World.h"
/// @defgroup haptics Haptics engine and haptic buffers
/// @brief Real-time haptics simulation and device interface


/// @ingroup haptics
/// @brief Haptics engine: real-time haptics loop, proxy projection, and force computation
class HapticEngine {

    public:
        /// @brief Construct a haptics engine with shared world and physics buffers.
        /// @param world Refernce to shared world state
        /// @param phys Refernce to shared physics command buffers
        HapticEngine(World& world, PhysicsBuffers& phys);

        /// @brief Destructor
        ~HapticEngine();

        /// @brief Main haptics loop (call in dedicated high-priority thread)
        /// @details Runs an infinite loop with 1ms timestep. Call stop on the thread to exit.
        void run(); // main haptics loop
        
        /// @brief  Read the latest haptics snapshot (for rendering or logging)
        /// @return Latest HapticsSnapshot
        HapticSnapshot readSnapshot() const { return bufs_.snapBuf.read(); }

        /// @brief Submit a new tool pose (from device) for processing.
        /// @param T_ws Tool pose in world space
        /// @param t_sec Timestamp in seconds (for logging)
        void submitToolPose(const Pose& T_ws, double t_sec) {
            ToolIn in = bufs_.inBuf.read();
            in.devicePose_ws = T_ws;
            in.t_sec = t_sec;
            bufs_.inBuf.write(in);
        }


    private:
        World& world_; ///< Shared world reference

        HapticsBuffers bufs_; ///< Haptics buffers
        Pose proxyPosePrev_{}; ///< Previous proxy pose (for velocity)
        PhysicsBuffers* physBufs_ = nullptr; ///< Shared physics buffers (for applying forces)


        // --- core loop functions ---
        /// @brief Update haptics state by one timestep
        /// @param dt Timestep in seconds (e.g. 0.001 for 1kHz)
        void update(float dt);
        void computeForces(const Pose& toolPose, const glm::dvec3& toolVel, bool toolButton,
                           Pose& proxyPose, glm::dvec3& force, Pose& refPose);
}; 
// main.cpp 
#include <glm/glm.hpp>
#include "viz/Window.h"
#include "world/world.h"
#include "scene/Scene.h"
#include <thread>
#include <chrono>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>          // NEW: for timeBeginPeriod/timeEndPeriod
#pragma comment(lib, "winmm.lib")
#include <immintrin.h>         // NEW: for _mm_pause (optional spin)
#endif

int main() {
    Window win({});
    World world;
    Scene  scene(win, world);

    // High-priority "haptics" thread
    std::jthread haptics([&](std::stop_token st) {
#ifdef _WIN32
        timeBeginPeriod(1); // NEW: better sleep granularity (undo at end)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
        using clock = std::chrono::steady_clock;
        constexpr auto period = std::chrono::microseconds(1000); // NEW: 1 kHz
        constexpr auto slack = std::chrono::microseconds(200);  // NEW: coarse sleep margin
        auto next = clock::now() + period;                       // NEW

        // Optional (but avoid I/O in loop)
        // std::cout.setf(std::ios::fixed);
        // std::cout.precision(2);

        while (!st.stop_requested()) {

            // ===== WORK =====
            const auto& env_ref = world.statics[0].env;
            Pose pose;
            readPose(world, 1, pose);
            if (env_ref->phi(pose.p) < 0.0) {
                pose.p = env_ref->project(pose.p);
                writePose(world, 2, pose);
            }
            else {
                Pose pose2;
                readPose(world, 2, pose2);        // NEW: was read into 'pose' by mistake
                if (pose.p != pose2.p) {
                    writePose(world, 2, pose);
                }
            }
            // =================

            // --- schedule next tick (NEW) ---
            auto now = clock::now();
            if (now > next + 5 * period) {        // if badly behind (e.g., debugger), resync
                next = now + period;
            }
            else {
                next += period;
            }

            // Sleep most of the remaining time
            now = clock::now();
            if (next - slack > now) {
                std::this_thread::sleep_until(next - slack);
            }

            // Final short spin to tighten accuracy
            while (clock::now() < next) {
#ifdef _WIN32
                _mm_pause();
#else
                std::this_thread::yield();
#endif
            }
        }

#ifdef _WIN32
        timeEndPeriod(1); // NEW: undo timer granularity change
#endif
        });

    scene.run();
    return 0;
}

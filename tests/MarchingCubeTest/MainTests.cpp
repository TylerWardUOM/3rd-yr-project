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
#endif
int main() {
    Window win({});
	World world;
    Scene  scene(win, world);


    // High - priority "haptics" thread that just prints mouse pos
        std::jthread haptics([&](std::stop_token st) {
#ifdef _WIN32
        // Conservative priority bump (avoid REALTIME/TIME_CRITICAL)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
        using clock = std::chrono::steady_clock;
        auto next = clock::now();

        // Optional: reduce i/o churn
        std::cout.setf(std::ios::fixed);
        std::cout.precision(2);

        while (!st.stop_requested()) {
            const auto& env_ref = world.statics[0].env;
            Pose pose;
            readPose(world, 1, pose);
            if (env_ref->phi(pose.p) < 0.0) {
                pose.p = env_ref->project(pose.p);
                writePose(world, 2, pose);
            }
            else{
                writePose(world, 2, pose);
            }
        }
            });



    scene.run();
    return 0;
}
   
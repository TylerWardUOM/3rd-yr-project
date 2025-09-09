// main.cpp 
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "viz/Window.h"
#include "world/world.h"
#include "viz/Camera.h"
#include "viz/GlSceneRenderer.h"
#include "scene/Scene.h"
#include "scene/ui/ViewportController.h"
#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"

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
    Window win({}); //Window Object

    Camera camera; // Camera Object

	World world; // World Object

	GlSceneRenderer renderer(camera); // Renderer Object

	ViewportController vpCtrl(win, world); // Viewport Controller Object

	Scene scene(win, world, renderer, camera); // Scene Object

	EntityId planeId = scene.addPlane({ {0,0,0}, {1.0,0,0,0} }, { 0.8f, 0.8f, 0.8f }); // Add a plane at origin
	EntityId sphereId = scene.addSphere({ {0,0.5,0}, {1.0,0,0,0} }, 0.1f, { 0.1f, 0.9f, 0.1f }); // Add a sphere above the plane
    EntityId sphereId2 = scene.addSphere({ {0,0.5,0}, {1.0,0,0,0} }, 0.1f, { 0.8f, 0.1f, 0.1f }); // Add a sphere above the plane
	scene.setSelected(sphereId); // Set drag target to the sphere entity
	world.publishSnapshot(0.0); // Initial publish to populate snapshot

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
			WorldSnapshot snap = world.readSnapshot();
			Pose planePose = snap.surfaces[world.findSurfaceIndexById(snap, planeId)].T_ws;
            // local frame normal
            glm::dvec3 n_local{ 0.0, 1.0, 0.0 };

            // rotate into world
            glm::dmat3 R = glm::mat3_cast(glm::quat(planePose.q));   // rotation matrix
            glm::dvec3 n_world = glm::normalize(R * n_local);

            // pick a world point on the plane (pose origin in this convention)
            glm::dvec3 p_world = planePose.p;

            // plane equation: n�x = d
            double d = glm::dot(n_world, p_world);

            // now construct your plane SDF
            PlaneEnv plane(n_world, d);
			Pose pose = snap.surfaces[world.findSurfaceIndexById(snap,sphereId)].T_ws;
            if (plane.phi(pose.p) < 0.0) {
                pose.p = plane.project(pose.p);
                world.setPose(sphereId2, pose);
				world.publishSnapshot(0.0); // publish updated state
            }
            else {
                Pose pose2 = snap.surfaces[world.findSurfaceIndexById(snap, sphereId2)].T_ws;
                if (pose.p != pose2.p) {
                    world.setPose(sphereId2, pose);
                    world.publishSnapshot(0.0); // publish updated sta                
                }
            }
            // =================

            // --- schedule next tick  ---
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
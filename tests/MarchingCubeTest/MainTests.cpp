// main.cpp 
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "viz/Window.h"
#include "world/World.h"
#include "viz/Camera.h"
#include "viz/GlSceneRenderer.h"
#include "scene/Scene.h"
#include "scene/ui/ViewportController.h"
#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"
#include "haptics/HapticEngine.h"
#include "physics/PhysicsEngine.h"
#include "physics/PhysXEngine.h"
#include "physics/PhysicsBuffers.h"
#include <physx/PxPhysicsAPI.h>
#include <thread>
#include <chrono>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>          // NEW: for timeBeginPeriod/timeEndPeriod
#pragma comment(lib, "winmm.lib")
#include <immintrin.h>         // NEW: for _mm_pause (optional spin)
#endif
#include <iostream>

int main() {
    Window win({}); //Window Object

    Camera camera; // Camera Object

	World world; // World Object

	GlSceneRenderer renderer(camera); // Renderer Object

    PhysicsBuffers buf; // Physics Buffers Object


    HapticEngine haptic(world, buf); // Haptic Engine Object

    PhysicsEnginePhysX physics(world, buf); // Physics Engine Object

    Scene scene(win, world, renderer, camera, haptic, physics); // Scene Object

    //Ground Plane
	EntityId planeId = scene.addPlane({ {0,0,0}, {1.0,0,0,0} }, { 0.8f, 0.8f, 0.8f }); // Add a plane at origin

    //Wall
    EntityId Plane2Id = scene.addPlane({ {0,0,-20}, {1.0,0,0,0} }, { 0.8f, 0.2f, 0.2f }); // Add a plane below the origin
	glm::dquat rot = glm::angleAxis(glm::radians(90.0), glm::dvec3(1,0,0));
    world.rotate(Plane2Id,rot); // Rotate the plane to be a wall
    EntityId Plane3Id = scene.addPlane({ {0,0,20}, {1.0,0,0,0} }, { 0.8f, 0.2f, 0.2f }); // Add a plane below the origin
	rot = glm::angleAxis(glm::radians(-90.0), glm::dvec3(1,0,0));
    world.rotate(Plane3Id,rot); // Rotate the plane to be a wall
    //Test Spheres
    EntityId sphereId = scene.addSphere({ {3.0,2.0,0}, {1.0,0,0,0} }, 0.3f, { 0.2f, 0.9f, 0.9f }); // Add a sphere above the plane
    EntityId sphereId2 = scene.addSphere({ {0,0,0}, {1.0,0,0,0} }, 0.3f, { 0.1f, 0.9f, 0.1f }); // Add a sphere above the plane
	
    scene.setSelected(planeId); // Set drag target to the sphere entity
	world.publishSnapshot(0.0); // Initial publish to populate snapshot

    PhysicsProps sphereprops = physics.getPhysicsProps(sphereId);
    sphereprops.restitution = 0.9; 
    physics.setPhysicsProps(sphereId, sphereprops); // Make sphere dynamic with some friction
    physics.rebuildActors(); // Rebuild actors to apply new properties

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
        haptic.run(); // Start the haptic engine
        });

    std::jthread physThread([&] (std::stop_token st) {
        using clock = std::chrono::steady_clock;
        using namespace std::chrono;

        const auto period = 4ms;                 // target: 1 kHz
        const auto slack  = 200us;               // sleep until (next - slack), then spin
        auto last = clock::now();
        auto next = last + period;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        last = clock::now();
        while (!st.stop_requested()) {
            // --- sleep-then-spin until the scheduled tick ---
            auto wake = next - slack;
            if (wake > clock::now())
                std::this_thread::sleep_until(wake);
            while (clock::now() < next) {
                std::this_thread::yield();       // or _mm_pause() on x86 for less jitter
            }

            // --- run step with measured dt ---
            const auto now  = clock::now();
            const double dt = duration<double>(now - last).count();  // seconds
            last = now;

            physics.step(dt);  // your engine can internally fixed-substep (e.g., 240 Hz)

            // schedule next tick; recover if we overran
            next += period;
            if (now > next + period) {           // fell far behind: resync
                next = now + period;
                last = now;
            }
        }
    });


	scene.run();
	return 0;
}
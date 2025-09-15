#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include "viz/Camera.h"       
#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"
#include "scene/Body.h"
#include "scene/ui/Ui.h"
#include "imgui.h"
#include "viz/UI/ImGuiLayer.h"
#include "scene/ui/ViewportController.h"
#include <iostream>
#include <cstdlib>

void processInput(GLFWwindow* window, Camera* cam, float dx, float dy, Body* bod);

int main() {
	Window win({}); //Window Object

	Camera camera; // Camera Object
	Shader shader("shaders/general.vert", "shaders/basic.frag"); //Shader Object
	Mesher mesher; // Mesher Object

	//Define ground plane
    auto prim_ground = std::make_unique<PlaneEnv>(glm::dvec3{ 0.0,1.0,0.0 }, 0.0);
    auto mesh_ground = std::make_unique<MeshGPU>();
    Body groundBody(std::move(prim_ground), std::move(mesh_ground), &mesher, &shader);
	groundBody.setMCBounds({ -10.0,-0.1,-10.0 }, { 10.0,0.1,10.0 }); //Region to mesh
	groundBody.remeshIfPossible();

	//Define test sphere
    auto prim_sphere = std::make_unique<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1);
    auto mesh_sphere = std::make_unique<MeshGPU>();
    Body sphereBody(std::move(prim_sphere), std::move(mesh_sphere), &mesher, &shader);

	ImGuiLayer imgui(win); 
    imgui.init(); // ImGui layer
    UI ui;

    UITransformState bodyState{};
    UICameraState    camState{};
    UISceneStats     stats{};
    UIPanelConfig    cfg{};
	UIControllerState controllerState{};

    // Controller setup
    ViewportController view_controller(win);
    view_controller.setCamera(&camera);
    view_controller.setDragTarget(&sphereBody);

	// UI scene commands
    UICommands cmds;
    //Body Commands
    cmds.setBodyPosition = [&](float x, float y, float z) {
        sphereBody.setPosition({ x, y, z });
        };
	// Camera Commands
    cmds.setCameraFov = [&](float fov) { camera.fovDeg = fov; };
    cmds.setCameraNear = [&](float n) { camera.znear = n;   };
    cmds.setCameraFar = [&](float f) { camera.zfar = f;   };
    cmds.setCameraAngles = [&](float yaw, float pitch) {
        camera.yawDeg = yaw; camera.pitchDeg = pitch;
        camera.clampPitch();
        camera.updateVectors();
        };

	// Viewport Controller Commands
    cmds.setMoveSpeed = [&](float v) { view_controller.setMoveSpeed(v); };
    cmds.setMouseSensitivity = [&](float v) { view_controller.setMouseSensitivity(v); };
    cmds.setScrollZoomSpeed = [&](float v) { view_controller.setScrollZoomSpeed(v); };
    cmds.setScrollZoomSpeed = [&](float v) { view_controller.setScrollZoomSpeed(v); };
    cmds.setInvertY = [&](bool  v) { view_controller.setInvertY(v); };
    cmds.setRmbToLook = [&](bool  v) { view_controller.setRmbToLook(v); };
    // hand off to UI once
    ui.setCommands(cmds);




    while (win.isOpen()) {
		// ---- Per-frame
		double sy = win.popScrollY(); // scroll Y (accumulated)
        if (sy != 0.0) view_controller.onScroll(sy);

        // ---- Input gated by ImGui capture
        const bool uiCapturing = imgui.wantCaptureMouse() || imgui.wantCaptureKeyboard();
        view_controller.update(/*dt=*/0.f, uiCapturing);

		// ---- Update state snapshot
        stats.fps = ImGui::GetIO().Framerate;
		
		controllerState.moveSpeed = view_controller.moveSpeed();
		controllerState.mouseSensitivity = view_controller.mouseSensitivity();
        bodyState.position = sphereBody.getPosition();

		// Project sphere out of ground if intersecting
        if (groundBody.primitive()->phi(bodyState.position) < 0.0) {
            sphereBody.setPosition(groundBody.primitive()->project(bodyState.position));
        }

		// ---- UI
		imgui.begin();
        ui.drawDebugPanel(stats);
		ui.drawBodyPanel(bodyState, cmds);
		ui.drawControllerPanel(controllerState, cmds);

        // ---- 3D render
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sphereBody.render(camera);
		groundBody.render(camera);

		// --- end frame
        imgui.end();
        win.swap();
        win.poll();
    }

    imgui.shutdown();
    return 0;
}
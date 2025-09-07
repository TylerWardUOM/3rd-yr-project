#include "viz/UI/ImGuiLayer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "viz/Shader.h"
#include "viz/Window.h"
#include "viz/Camera.h"        // <-- ensure Camera is included
#include "env/primitives/SphereEnv.h"
#include "env/primitives/PlaneEnv.h"
#include "scene/Body.h"
#include "scene/ui/Ui.h"
#include "imgui.h"
#include "scene/ui/ViewportController.h"
#include <iostream>
#include <cstdlib>

// If processInput isn't defined elsewhere, keep this prototype + the function below
void processInput(GLFWwindow* window, Camera* cam, float dx, float dy, Body* bod);

int main() {
    Window win({});

    Camera camera;
    Shader shader("shaders/general.vert", "shaders/basic.frag");
    Mesher mesher;
	//Define ground plane
    auto prim_ground = std::make_unique<PlaneEnv>(glm::dvec3{ 0.0,1.0,0.0 }, 0.0);
    auto mesh_ground = std::make_unique<MeshGPU>();
    Body groundBody(std::move(prim_ground), std::move(mesh_ground), &mesher, &shader);
	//Define test sphere
    auto prim_sphere = std::make_unique<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1);
    auto mesh_sphere = std::make_unique<MeshGPU>();
    Body sphereBody(std::move(prim_sphere), std::move(mesh_sphere), &mesher, &shader);

    ImGuiLayer imgui; imgui.init(win.handle(), "#version 330");
    UI ui;

    UITransformState bodyState{};
    UICameraState    camState{};
    UISceneStats     stats{};
    UIPanelConfig    cfg{};
	UIControllerState controllerState{};

    // Controller setup
    ViewportController view_controller(win.handle());
    view_controller.setCamera(&camera);
    view_controller.setDragTarget(&sphereBody);

    UICommands cmds;
    cmds.setBodyPosition = [&](float x, float y, float z) {
        sphereBody.setPosition({ x, y, z });
        };
    cmds.setCameraFov = [&](float fov) { camera.fovDeg = fov; };
    cmds.setCameraNear = [&](float n) { camera.znear = n;   };
    cmds.setCameraFar = [&](float f) { camera.zfar = f;   };
    cmds.setCameraAngles = [&](float yaw, float pitch) {
        camera.yawDeg = yaw; camera.pitchDeg = pitch;
        camera.clampPitch();
        camera.updateVectors();
        };

    cmds.setMoveSpeed = [&](float v) { view_controller.setMoveSpeed(v); };
    cmds.setMouseSensitivity = [&](float v) { view_controller.setMouseSensitivity(v); };
    cmds.setScrollZoomSpeed = [&](float v) { view_controller.setScrollZoomSpeed(v); };
    cmds.setScrollZoomSpeed = [&](float v) { view_controller.setScrollZoomSpeed(v); };
    cmds.setInvertY = [&](bool  v) { view_controller.setInvertY(v); };
    cmds.setRmbToLook = [&](bool  v) { view_controller.setRmbToLook(v); };
    // hand off to UI once
    ui.setCommands(cmds);


    // viewport setup ...
    int fbw = 0, fbh = 0; glfwGetFramebufferSize(win.handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    // mouse state ...
    bool firstMouse = true;
    double lastX = 0.0, lastY = 0.0;


    while (win.isOpen()) {
        double x, y;
        glfwGetCursorPos(win.handle(), &x, &y);

        if (firstMouse) { firstMouse = false; lastX = x; lastY = y; }
        double dx = x - lastX;
        double dy = lastY - y; // invert Y
        lastX = x; lastY = y;

        stats.fps = ImGui::GetIO().Framerate;
		bodyState.position = sphereBody.getPosition();
        if (groundBody.primitive()->phi(bodyState.position) < 0.0) {
			sphereBody.setPosition(groundBody.primitive()->project(bodyState.position));
        }
		controllerState.moveSpeed = view_controller.moveSpeed();
		controllerState.mouseSensitivity = view_controller.mouseSensitivity();
         
		imgui.begin();
        ui.drawDebugPanel(stats);
		ui.drawBodyPanel(bodyState, cmds);
		ui.drawControllerPanel(controllerState, cmds);

        // ---- Input gated by ImGui capture
        const bool uiCapturing = imgui.wantCaptureMouse() || imgui.wantCaptureKeyboard();

        // ---- 3D render
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sphereBody.render(camera);
		groundBody.render(camera);
        view_controller.update(/*dt=*/0.f, uiCapturing);


        // ---- Render UI on top
        imgui.end();

        win.swap();
        win.poll();
    }

    imgui.shutdown();
    return 0;
}

// Minimal input (same as your earlier version)
void processInput(GLFWwindow* window, Camera* cam, float dx, float dy, Body* bod)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam->eye += cam->front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam->eye -= cam->front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam->eye -= cam->right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam->eye += cam->right * speed;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) bod->rotate(0.01f, { 0.0f,1.0f,0.0f });

    //cam->addYaw(dx);
    //cam->addPitch(dy);
    cam->clampPitch();
    cam->updateVectors();
}

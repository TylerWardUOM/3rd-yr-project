#include "scene/Scene.h"
#include <vector>
#include <iostream>

Scene::Scene(Window& win, World& world, ISceneRenderer& renderer, Camera& cam):
    win_(win),
    world_(world),
    renderer_(renderer),
    cam_(cam),
    imgui_(win),
    ui_(),
    vpCtrl_(win, world)
{
    init_Bodies();
    init_Ui();

    // Camera defaults
    cam_.eye    = {0.0f, 1.0f, 1.0f};
    cam_.up     = {0.0f, 1.0f, 0.0f};
    cam_.fovDeg = 60.f;
    cam_.yawDeg   = 0.f;
    cam_.pitchDeg = -45.f;

    // Viewport controller
    vpCtrl_.setCamera(&cam_);
    vpCtrl_.setDragTarget(2);

    // Initial viewport size
    int fbw=0, fbh=0; 
    win_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam_.aspect = float(fbw)/float(fbh); 
        vpCtrl_.setViewport(fbw, fbh); 
    }

    // Initial Camera vectors
    cam_.updateVectors();

}

Scene::~Scene() {
    imgui_.shutdown();
}

void Scene::run() {
    double last = 0.0;
    while (win_.isOpen()) {
        // timing 
        double now;
        win_.getTime(now);
        float dt = float(now - last);
        last = now;

        // dispatch scroll
        double sy = win_.popScrollY();
        if (sy != 0.0) vpCtrl_.onScroll(sy);

        // UI capture gates controller
        bool uiCapturing = imgui_.wantCaptureMouse() || imgui_.wantCaptureKeyboard();

        update(dt, uiCapturing);
        render();

        win_.swap();
        win_.poll();
    }
}

void Scene::update(float /*dt*/, bool uiCapturing) {

    // Keep camera aspect + viewport in sync
    int fbw=0, fbh=0; win_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam_.aspect = float(fbw)/float(fbh);
        glViewport(0,0,fbw,fbh); // or win_.setViewport wrapper
    }

    // Controllers (keyboard/mouse)
    vpCtrl_.update(/*dt*/0.f, uiCapturing);

    // UI snapshots (minimal)
    imgui_.getFps(stats_.fps);
    ctrlState_.moveSpeed        = vpCtrl_.moveSpeed();
    ctrlState_.mouseSensitivity = vpCtrl_.mouseSensitivity();
    camState_.pitchDeg  = cam_.pitchDeg;
    camState_.position  = cam_.eye;
    camState_.yawDeg  = cam_.yawDeg;



    double mx=0.0, my=0.0;
    win_.getCursorPos(mx, my);          // GLFW — safe on render thread
    //world_.writeMouse(mx, my);    // publish for haptics

}

void Scene::render() {
    renderer_.setViewProj(cam_.view(), cam_.proj());
    renderer_.submit(world_.readSnapshot(), HapticsVizSnapshot{});
    renderer_.render();
    // UI
    imgui_.begin();
    ui_.drawDebugPanel(stats_);
    ui_.drawBodyPanel(bodyState_);
    ui_.drawControllerPanel(ctrlState_);
    ui_.drawCameraPanel(camState_);
    imgui_.end();
}


void Scene::init_Bodies(){
    entity_ids_.push_back(world_.addPlane(Pose{glm::dvec3{0.0,0.0,0.0}, glm::dquat{1.0,0.0,0.0,0.0}})); // ground plane
    entity_ids_.push_back(world_.addSphere(Pose{glm::dvec3{0.0,0.5,0.0}, glm::dquat{1.0,0.0,0.0,0.0}}, 0.1)); // small sphere
    world_.publishSnapshot(0.0);
    // body_ids_.push_back(addBody(world_,
    //         std::make_shared<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 2.0),
    //         &shader_,
    //         MCParams{ .minB={-10.0,-10.0,-10.0}, .maxB={10.0,10.0,10.0}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    // ));
    // body_ids_.push_back(addBody(world_,
    //         std::make_shared<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1),
    //         &shader_,
    //         MCParams{ .minB={-1.5,-1.5,-1.5}, .maxB={1.5,1.5,1.5}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    // ));

    //     body_ids_.push_back(addBody(world_,
    //         std::make_shared<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1),
    //         &redShader_,
    //         MCParams{ .minB={-1.5,-1.5,-1.5}, .maxB={1.5,1.5,1.5}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    // ));
    //sphere_.setPosition({0.0f, 0.5f, 0.0f});

}

void Scene::init_Ui(){
    // --- Bind UI commands ---
    UICommands cmds;

    // Body commands
    cmds.setBodyPosition = [&](float x, float y, float z) {
        //sphere_.setPosition({x, y, z});
    };

    // Camera commands
    cmds.setCameraFov = [&](float fov) { cam_.fovDeg = fov; };
    cmds.setCameraNear = [&](float n) { cam_.znear = n; };
    cmds.setCameraFar  = [&](float f) { cam_.zfar = f; };
    cmds.setCameraAngles = [&](float yaw, float pitch) {
        cam_.yawDeg = yaw;
        cam_.pitchDeg = pitch;
        cam_.clampPitch();
        cam_.updateVectors();
    };

    // ViewportController commands
    cmds.setMoveSpeed        = [&](float v) { vpCtrl_.setMoveSpeed(v); };
    cmds.setMouseSensitivity = [&](float v) { vpCtrl_.setMouseSensitivity(v); };
    cmds.setScrollZoomSpeed  = [&](float v) { vpCtrl_.setScrollZoomSpeed(v); };
    cmds.setInvertY          = [&](bool v)  { vpCtrl_.setInvertY(v); };
    cmds.setRmbToLook        = [&](bool v)  { vpCtrl_.setRmbToLook(v); };

    // Hand off once
    ui_.setCommands(cmds);
}
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
    init_Ui();

    // Viewport controller
    vpCtrl_.setCamera(&cam_);

    // Initial viewport size
    int fbw=0, fbh=0; 
    win_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam_.aspect = float(fbw)/float(fbh); 
        vpCtrl_.setViewport(fbw, fbh); 
    }

    // Camera defaults
    cam_.eye    = {0.0f, 1.0f, 1.0f};
    cam_.up     = {0.0f, 1.0f, 0.0f};
    cam_.fovDeg = 60.f;
    cam_.yawDeg   = 0.f;
    cam_.pitchDeg = -45.f;

    // Initial Camera vectors
    cam_.updateVectors();

}

Scene::~Scene() {
    imgui_.shutdown();
}

void Scene::run() {
    double last = 0.0;
    while (win_.isOpen()) {
        // --- Timing 
        double now;
        win_.getTime(now);
        float dt = float(now - last);
        last = now;
        // UI capture gates controller
        bool uiCapturing = imgui_.wantCaptureMouse() || imgui_.wantCaptureKeyboard();
        // --- Update + Render ---
        update(dt, uiCapturing);
        render();
        // --- Swap + Poll ---
        win_.swap();
        win_.poll();
        world_.publishSnapshot(now); // publish once per frame
    }
}

void Scene::update(float /*dt*/, bool uiCapturing) {

    // Keep camera aspect + viewport in sync
    int fbw=0, fbh=0; win_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam_.aspect = float(fbw)/float(fbh);
        renderer_.onResize(fbw, fbh);
    }


    // Controllers (keyboard/mouse)
    vpCtrl_.update(/*dt*/0.f, uiCapturing, selected_);
    // Scroll
    double sy = win_.popScrollY();
    if (sy != 0.0) vpCtrl_.onScroll(sy);

    // Update UI snapshots (minimal)
    imgui_.getFps(stats_.fps);
    ctrlState_.moveSpeed        = vpCtrl_.moveSpeed();
    ctrlState_.mouseSensitivity = vpCtrl_.mouseSensitivity();
    camState_.pitchDeg  = cam_.pitchDeg;
    camState_.position  = cam_.eye;
    camState_.yawDeg  = cam_.yawDeg;
    WorldSnapshot snap = world_.readSnapshot();
    int selectedIdx = world_.findSurfaceIndexById(snap, selected_);
    Pose selectedPose = snap.surfaces[selectedIdx].T_ws;
    //std::cout << "Selected position: " << selectedPose.p.x << ", " << selectedPose.p.y << ", " << selectedPose.p.z << std::endl;
    bodyState_.position = glm::vec3(selectedPose.p);


    // -- temp Mouse position (for haptics) temp ---
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



EntityId Scene::addPlane(Pose pose, glm::vec3 colour){
    return world_.addPlane(pose, colour);
}

EntityId Scene::addSphere(Pose pose, float radius, glm::vec3 colour){
    return world_.addSphere(pose, radius, colour);
}

void Scene::init_Ui(){
    // --- Bind UI commands ---
    UICommands cmds;

    // Body commands
    cmds.setBodyPosition = [&](float x, float y, float z) {
        WorldSnapshot snap = world_.readSnapshot();
        Pose selectedPose = snap.surfaces[selected_].T_ws;
        world_.setPose(selected_, {(glm::dvec3({x,y,z})),selectedPose.q});
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
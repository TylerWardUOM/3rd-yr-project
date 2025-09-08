#include "scene/Scene.h"
#include <vector>
#include <iostream>

Scene::Scene(Window& win, World& world):
    win_(win),
    world_(world),
    cam_(),
    shader_("shaders/general.vert","shaders/basic.frag"),
    mesher_(),
    imgui_(win),
    ui_(),
    vpCtrl_(win, world)
{
    // ground_(std::make_unique<PlaneEnv>(glm::dvec3{ 0.0,1.0,0.0 }, 0.0),
    //         std::make_unique<MeshGPU>(),
    //         &mesher_,
    //         &shader_
    // ),
    // sphere_(std::make_unique<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1),
    //         std::make_unique<MeshGPU>(),
    //         &mesher_,
    //         &shader_
    // )
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
    vpCtrl_.setDragTarget(1);

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
    Pose bodyPose;
    readPose(world_,1,bodyPose);

    // // Constraint: keep sphere above plane temp logic will most likely move to haptic loop
    // glm::dvec3 p = sphere_.getPosition();
    // if (ground_.primitive()->phi(p) < 0.0) {
    //     sphere_.setPosition(ground_.primitive()->project(p));
    // }
    double mx=0.0, my=0.0;
    win_.getCursorPos(mx, my);          // GLFW — safe on render thread
    //world_.writeMouse(mx, my);    // publish for haptics

}

void Scene::render() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3D
    for (BodyId id : body_ids_) {
        Pose pose;
        if (readPose(world_, id, pose)) {
            // build model matrix from pose, bind W.renderables[id], draw
            const glm::dmat4 T = glm::translate(glm::dmat4(1.0), pose.p);     // p is dvec3
            const glm::dmat4 R = glm::mat4_cast(pose.q);                       // q is dquat
            glm::dmat4 model = T * R;
            world_.renderables[id].render(cam_, model);
        }
    }

    // UI
    imgui_.begin();
    ui_.drawDebugPanel(stats_);
    ui_.drawBodyPanel(bodyState_);
    ui_.drawControllerPanel(ctrlState_);
    ui_.drawCameraPanel(camState_);
    imgui_.end();
}


void Scene::init_Bodies(){
    body_ids_.push_back(addBody(world_,
            std::make_shared<PlaneEnv>(glm::dvec3{ 0.0,1.0,0.0 }, 0.0),
            &shader_,
            MCParams{ .minB={-10.0,-0.1,-10.0}, .maxB={10.0,0.1,10.0}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    ));
    body_ids_.push_back(addBody(world_,
            std::make_shared<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1),
            &shader_,
            MCParams{ .minB={-1.5,-1.5,-1.5}, .maxB={1.5,1.5,1.5}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    ));
        body_ids_.push_back(addBody(world_,
            std::make_shared<SphereEnv>(glm::dvec3{ 0.0,0.0,0.0 }, 0.1),
            &shader_,
            MCParams{ .minB={-1.5,-1.5,-1.5}, .maxB={1.5,1.5,1.5}, .nx=64, .ny=64, .nz=64, .iso=0.0 }
    ));
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
#include "scene/Scene.h"
#include <vector>
#include <iostream>

/**
 * @details
 * The Scene constructor initializes the scene with references to the window, world, renderer, camera, and haptic engine. It sets up the UI, viewport controller, and camera defaults. The initial viewport size is obtained from the window's framebuffer size, and the camera's aspect ratio is set accordingly. The camera's position, orientation, and field of view are also initialized.
 * 
 */
Scene::Scene(Window& win, World& world, ISceneRenderer& renderer, Camera& cam, HapticEngine& haptic/*temp for mouse pos*/, PhysicsEnginePhysX& physics):
    win_(win),
    world_(world),
    renderer_(renderer),
    cam_(cam),
    imgui_(win),
    ui_(),
    vpCtrl_(win, haptic),
    haptic_(haptic), // temp for mouse pos
    physics_(physics)
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

/**
 * @details
 * - The run method implements the main application loop, which continues until the window is closed. 
 * - It handles timing, input capture for UI interactions, scene updates, rendering, and event polling. 
 * - The loop calculates the time delta between frames to ensure smooth updates. 
 * - It checks if the UI is capturing mouse or keyboard input to prevent conflicts with scene controls. 
 * - The update method is called to process scene logic, followed by the render method to draw the current state. 
 * - Finally, the window's buffers are swapped and events are polled, and a new world snapshot is published for synchronization.
 */
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

/**
 * @details
 * The update method performs several key functions to maintain the scene's state:
 * - It synchronizes the camera's aspect ratio and viewport size with the window's framebuffer dimensions.
 * - It updates the viewport controller based on user input, including handling mouse scroll events.
 * - It refreshes UI elements with the latest statistics, camera state, and controller settings.
 * - It reads the current world snapshot to update the state of the selected entity, including its position and color.
 * - It rebuilds the list of entity options for the UI, ensuring that the selection state is consistent with the application's current state.
 */
void Scene::update(float /*dt*/, bool uiCapturing) {

    // Keep camera aspect + viewport in sync
    int fbw=0, fbh=0; win_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        cam_.aspect = float(fbw)/float(fbh);
        renderer_.onResize(fbw, fbh);
    }


    // Controllers (keyboard/mouse)
    vpCtrl_.update(/*dt*/0.f, uiCapturing);
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
    //if (selected_ == world_.entityFor(Role::Tool)) world_.setToolPose(selectedPose); // temp for haptics
    bodyState_.colour   = glm::vec3(snap.surfaces[selectedIdx].colour);
    // Build options each frame
    struct Option { World::EntityId id; std::string label; };
    std::vector<Option> options;

    // Rebuild the options list each frame (cheap, small)
    bodyState_.entityOptions.clear();
    bodyState_.entityOptions.reserve(snap.numSurfaces);
    for (uint32_t i = 0; i < snap.numSurfaces; ++i) {
        const auto& s = snap.surfaces[i];
        bodyState_.entityOptions.push_back(s.id);
    }

    // Keep the UI state in sync with app selection
    bodyState_.selectedEntityId = selected_;   // std::optional<uint32_t>

    bodyState_.physicsProps=physics_.getPhysicsProps(selected_);




    // -- temp Mouse position (for haptics) temp ---
    double mx=0.0, my=0.0;
    win_.getCursorPos(mx, my);          // GLFW — safe on render thread
    //world_.writeMouse(mx, my);    // publish for haptics

}

/**
 * @details
 * The render method is responsible for drawing the current state of the scene. 
 * - It sets the view and projection matrices in the renderer based on the camera's current view and projection. 
 * - It then submits the latest world and haptic snapshots to the renderer for rendering.
 * - After rendering the scene, it begins a new ImGui frame, draws various UI panels (debug, body, controller, camera), and ends the ImGui frame.
 */
void Scene::render() {
    renderer_.setViewProj(cam_.view(), cam_.proj());
    renderer_.submit(world_.readSnapshot(), haptic_.readSnapshot());
    renderer_.render();
    // UI
    imgui_.begin();
    ui_.drawDebugPanel(stats_);
    ui_.drawBodyPanel(bodyState_);
    ui_.drawControllerPanel(ctrlState_);
    ui_.drawCameraPanel(camState_);
    imgui_.end();
}


/**
 * @details
 * The addPlane method adds a plane surface to the scene by calling the world's addPlane method with the specified pose and color. It returns the entity ID of the newly added plane.
 */
EntityId Scene::addPlane(Pose pose, glm::vec3 colour){
    return world_.addPlane(pose, colour);
}

/**
 * @details
 * The addSphere method adds a sphere surface to the scene by calling the world's addSphere method with the specified pose, radius, and color. It returns the entity ID of the newly added sphere.
 */
EntityId Scene::addSphere(Pose pose, float radius, glm::vec3 colour){
    return world_.addSphere(pose, radius, colour);
}

/**
 * @details
 * Binds UI commands to their respective functions in the scene, camera, and viewport controller.
 */
void Scene::init_Ui(){
    // --- Bind UI commands ---
    UICommands cmds;

    // Body commands
    cmds.setBodyPosition = [&](float x, float y, float z) { // fix to use index
        WorldSnapshot snap = world_.readSnapshot();
        int selectedIndx = world_.findSurfaceIndexById(snap, selected_);
        Pose selectedPose = snap.surfaces[selectedIndx].T_ws;
        //std::cout << "Old position: " << selectedPose.p.x << ", " << selectedPose.p.y << ", " << selectedPose.p.z << std::endl;
        //std::cout << "Setting body position to: " << x << ", " << y << ", " << z << std::endl;

        // --- Update world debug use only ---
        world_.setPose(selected_, {(glm::dvec3({x,y,z})),selectedPose.q});
        world_.setDirtyDebug(true);
        world_.publishSnapshot(12.0);

    };
    cmds.setBodyColour = [&](float r, float g, float b) {
        WorldSnapshot snap = world_.readSnapshot();
        int selectedIdx = world_.findSurfaceIndexById(snap, selected_);

        // --- Update world debug use only ---
        world_.setColour(selected_,{r,g,b});
        world_.publishSnapshot(15.0);
        
    };

    cmds.setSelectedEntity = [&](EntityId entityId) {
        selected_ = entityId;
    };

    cmds.setBodyDensity = [&](float density) {
        physics_.setDensity(selected_, density);
    };

    cmds.setBodyDynamic = [&](bool dynamic) {
        physics_.setDynamic(selected_, dynamic);
    };

    cmds.setBodyLinDamping = [&](float linDamping) {
        physics_.setLinDamping(selected_, linDamping);
    };

    cmds.setBodyAngDamping = [&](float angDamping) {
        physics_.setAngDamping(selected_, angDamping);
    };

    cmds.setBodyStaticFriction = [&](float staticFriction) {
        physics_.setStaticFriction(selected_, staticFriction);
    };

    cmds.setBodyDynamicFriction = [&](float dynamicFriction) {
        physics_.setDynamicFriction(selected_, dynamicFriction);
    };

    cmds.setBodyRestitution = [&](float restitution) {
        physics_.setRestitution(selected_, restitution);
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
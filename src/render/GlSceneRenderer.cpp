#include <memory>
#include "render/GlSceneRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render/shader/MVPUniforms.h"
#include <glm/gtc/matrix_access.hpp>
#include "geometry/GeometryDatabase.h"
#include "util/RobotUtils.h"
#include <iostream>

// --- utils ---
// build model matrix from Pose
glm::mat4 GlSceneRenderer::compose(const Pose& T) {
    // Rotation
    glm::mat4 R = glm::mat4_cast(glm::normalize(glm::quat(T.q)));

    // Uniform scale
    glm::mat4 S = glm::scale(glm::mat4(1.0f),
                             glm::vec3(static_cast<float>(T.s)));

    // Translation
    glm::mat4 M = R * S;
    M[3] = glm::vec4(glm::vec3(T.p), 1.0f);

    return M;
}


// ========== ctor/dtor ==========
GlSceneRenderer::GlSceneRenderer(
    Window& window,
    const GeometryDatabase& geomDb,
    RenderMeshRegistry& meshRegistry,
    msg::Channel<WorldCommand>& worldCmds,
    msg::Channel<ToolStateMsg>& toolState,
    msg::Channel<HapticSnapshotMsg>& hapticSnaps,
    msg::SnapshotChannel<WorldSnapshot>& worldSnaps
)
    : window_(window)
    , camera_()
    , imguiLayer_(window_, "#version 330")
    , ui_()
    , viewportCtrl_(window_, camera_, toolState, hapticSnaps)
    , geometryDb_(geomDb)
    , meshRegistry_(meshRegistry)
    , worldCmds_(worldCmds)
    , toolState_(toolState)
    , hapticSnaps_(hapticSnaps)
    , worldSnaps_(worldSnaps)
    , shader_("shaders/general.vert", "shaders/general.frag")
{
    ensurePrimitiveTemplates();

    // ---- Camera defaults ----
    camera_.eye      = {0.0f, 1.0f, 1.0f};
    camera_.up       = {0.0f, 1.0f, 0.0f};
    camera_.fovDeg   = 60.f;
    camera_.yawDeg   = 0.f;
    camera_.pitchDeg = -45.f;
    camera_.znear    = 0.01f;
    camera_.zfar     = 100.0f;
    camera_.updateVectors();

    // ---- Initial framebuffer size ----
    int fbw = 0, fbh = 0;
    window_.getFramebufferSize(fbw, fbh);
    if (fbw > 0 && fbh > 0) {
        camera_.aspect = float(fbw) / float(fbh);
        fbw_ = fbw;
        fbh_ = fbh;
    }

    
    initUICommands();
}

GlSceneRenderer::~GlSceneRenderer() {
    imguiLayer_.shutdown();
}

// ========== ISceneRenderer impl ==========
void GlSceneRenderer::ensurePrimitiveTemplates() {
    // if (unitSphere_.isUploaded() == 0) createUnitSphere(unitSphere_);
    // if (unitPlane_.isUploaded()  == 0) createUnitPlane(unitPlane_);
    // if (unitCylinder_.isUploaded() == 0) createUnitCylinder(unitCylinder_, 32);
}

// void GlSceneRenderer::registerTriMesh(MeshId id,
//                                       const float* pn, size_t pnCount,
//                                       const uint32_t* idx, size_t idxCount)
// {
//     auto [it, inserted] = triMeshes_.try_emplace(id);  // default-construct MeshGPU if new
//     it->second.upload(std::vector<float>(pn, pn + pnCount), std::vector<uint32_t>(idx, idx + idxCount)); // maybe replace tempoary vectors with pointers in MeshGPU::upload
// }

void GlSceneRenderer::onResize(int width, int height) {
    fbw_ = width; fbh_ = height;
    glViewport(0, 0, fbw_, fbh_);
}

void GlSceneRenderer::setViewProj(const glm::mat4& V, const glm::mat4& P) {
    V_ = V; P_ = P;
}

// void GlSceneRenderer::submit(const WorldSnapshot& world,
//                              const HapticSnapshot& haptic) {
//     world_ = world; // POD copy
//     haptic_   = haptic;
// }


void GlSceneRenderer::render() {
    // -- Handle resize --
    int fbw = 0, fbh = 0;
    window_.getFramebufferSize(fbw, fbh);
    if (fbw > 0 && fbh > 0) {
        camera_.aspect = float(fbw) / float(fbh);
        fbw_ = fbw;
        fbh_ = fbh;
    }

    // ---- Drain latest world snapshot ----
    worldSnaps_.tryRead(latestWorld_, worldSnapVersion_);

    // basic Gl state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    for (const ObjectState& obj : latestWorld_.objects) {

        // --- Geometry lookup ---
        const GeometryEntry* geom = &geometryDb_.get(obj.geom);
        if (!geom) continue;

        // --- Mesh lookup ---
        const MeshGPU* mesh = meshRegistry_.get(geom->renderMesh);
        if (!mesh) continue;

        // --- Build transient renderable ---
        Renderable r;
        r.mesh   = mesh;
        r.shader = &shader_;
        r.colour = obj.colourOverride;
        if (geom->type == SurfaceType::Plane) {
            r.useGridLines = 1.0;
        }

        // --- Render ---
        r.render(camera_, compose(obj.T_ws));
    }

    HapticSnapshotMsg hs;
    while (hapticSnaps_.tryConsume(hs)) {
        latestHaptics_ = std::move(hs);
    }

    // Need to Clean this COde Up Later

    // Pose proxyPose = latestHaptics_.proxyPose_ws;

    // const GeometryEntry* geom = &geometryDb_.get(2);
    // const MeshGPU* mesh = meshRegistry_.get(geom->renderMesh);
    // glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3((float)0.02));

    // Renderable r;
    // r.mesh   = mesh;
    // r.shader = &shader_;
    // r.colour = {0.8f, 0.2f, 0.2f};
    // // --- Render ---
    // r.render(camera_, compose(proxyPose) * S);

    // Pose devicePose = latestHaptics_.devicePose_ws;
    // r.mesh   = mesh;
    // r.shader = &shader_;
    // r.colour = {0.2f, 0.8f, 0.2f};
    // // --- Render ---
    // r.render(camera_, compose(devicePose) * S);

    drawOverlays();

    // End of bad chunk
    bool uiCapturing = imguiLayer_.wantCaptureMouse() || imguiLayer_.wantCaptureKeyboard();

    viewportCtrl_.update(0.0f, uiCapturing); // no dt, no UI capture
    double sy = window_.popScrollY();
    if (sy != 0.0) viewportCtrl_.onScroll(sy);

;
    imguiLayer_.begin();
    buildUIState(latestWorld_);
    // ui_.drawBodyPanel(bodyState_);
    imguiLayer_.getFps(stats_.fps);
    ui_.drawDebugPanel(stats_);
    ui_.drawCameraPanel(camState_);
    ui_.drawControllerPanel(ctrlState_);
    ui_.drawBodyPanel(bodyState_);
    imguiLayer_.end();
    window_.swap();
    window_.poll();
    
}

inline glm::dvec3 getTranslation(const glm::mat4& M) {
    return glm::vec3(M[3]); // xyz of 4th column
}

void GlSceneRenderer::drawOverlays() {
    // ------------------------------------------------------------
    // Get debug meshes from registry
    // ------------------------------------------------------------
    const RenderMeshHandle sphereHandle = meshRegistry_.getOrCreate(MeshKind::Sphere);
    const RenderMeshHandle cubeHandle   = meshRegistry_.getOrCreate(MeshKind::Cube);

    const MeshGPU* sphereMesh = meshRegistry_.get(sphereHandle);
    const MeshGPU* cubeMesh   = meshRegistry_.get(cubeHandle);

    if (!sphereMesh || !cubeMesh) return;

    auto renderDebugMesh = [&](const MeshGPU* mesh, const glm::mat4& M, const glm::vec3& colour) {
        Renderable r;
        r.mesh   = mesh;
        r.shader = &shader_;
        r.colour = colour;
        r.render(camera_, M);
    };

    auto sphereModel = [](const glm::dvec3& p, float radius) {
        glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(p));
        M = M * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
        return M;
    };

    auto poseToMat4 = [this](const Pose& pose) {
        return compose(pose);
    };

    auto linkCubeModel = [poseToMat4](const glm::dvec3& a,
                                    const glm::dvec3& b,
                                    float radius) {
        Pose linkPose = linkPoseBetween(a, b);
        const float L = static_cast<float>(glm::length(b - a));

        glm::mat4 M = poseToMat4(linkPose);

        glm::mat4 S_local = glm::scale(glm::mat4(1.0f),
                                    glm::vec3(radius, L, radius));

        // shift unit cube from [-0.5,0.5] to [0,1] in local Y
        glm::mat4 T_local = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(0.0f, 0.5f, 0.0f));

        return M * S_local * T_local;
    };

    // ------------------------------------------------------------
    // Ghost markers
    // ------------------------------------------------------------
    renderDebugMesh(
        sphereMesh,
        sphereModel(latestHaptics_.devicePose_ws.p, 0.016f),
        {0.1f, 1.0f, 0.1f}
    );

    renderDebugMesh(
        sphereMesh,
        sphereModel(latestHaptics_.proxyPose_ws.p, 0.015f),
        {0.1f, 0.1f, 1.0f}
    );

    // ------------------------------------------------------------
    // Robot overlay from measured joint angles
    // ------------------------------------------------------------
    basicRobot robot = {0.15, 0.15};

    glm::dvec3 angles;
    RobotState state = inverseKinematics(robot, glm::vec3(latestHaptics_.proxyPose_ws.p), angles);

    // const double q1 = latestHaptics_.latestAngles[0];
    // const double q2 = latestHaptics_.latestAngles[1];

    glm::dvec3 p0(0.0, 0.0, 0.0);

    auto Ts = forwardExplicitAll(robot.link1, robot.link2, angles);

    glm::dvec3 p1 = glm::dvec3(Ts[2][3]); // elbow
    glm::dvec3 p2 = glm::dvec3(Ts[3][3]); // end effector

    // ------------------------------------------------------------
    // Links
    // ------------------------------------------------------------
    const glm::vec3 linkColour  = {0.85f, 0.85f, 0.85f};
    const glm::vec3 jointColour = {1.0f, 1.0f, 0.1f};

    renderDebugMesh(cubeMesh, linkCubeModel(p0, p1, 0.010f), linkColour);
    renderDebugMesh(cubeMesh, linkCubeModel(p1, p2, 0.010f), linkColour);

    // ------------------------------------------------------------
    // Joints
    // ------------------------------------------------------------
    renderDebugMesh(sphereMesh, sphereModel(p0, 0.010f), jointColour);
    renderDebugMesh(sphereMesh, sphereModel(p1, 0.010f), jointColour);
    renderDebugMesh(sphereMesh, sphereModel(p2, 0.010f), jointColour);
}


// ========== helpers: draw ==========
void GlSceneRenderer::drawMeshRenderable(const MeshGPU& m, const glm::mat4& M, const glm::vec3& colour) {
    Renderable r;
    r.mesh = &m;
    r.shader = &shader_;
    r.colour = colour;
    r.render(camera_, M);
}

// void GlSceneRenderer::drawPlaneRenderable(const glm::mat4& M, const glm::vec2& halfExtents, const glm::vec3& colour) {
//     glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(halfExtents.x*2.f, 1.f, halfExtents.y*2.f));
//     Renderable r;
//     r.mesh = &unitPlane_;  
//     r.shader = &shader_;
//     r.colour = colour;
//     r.useGridLines = 1.0; // show grid lines for planes
//     r.render(camera_, M * S);
// }


// void GlSceneRenderer::drawSphereRenderable(const glm::mat4& M, double radius, const glm::vec3& colour) {
//     glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3((float)radius));
//     Renderable r;
//     r.mesh = &unitSphere_; 
//     r.shader = &shader_;
//     r.colour = colour;
//     r.render(camera_, M * S);}


// void GlSceneRenderer::drawCylinderRenderable(const glm::mat4& M, double radius, double height, const glm::vec3& colour) {
//     glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3((float)radius, (float)height, (float)radius));
//     Renderable r;
//     r.mesh = &unitCylinder_; 
//     r.shader = &shader_;
//     r.colour = colour;
//     r.render(camera_, M * S);
// }



void GlSceneRenderer::initUICommands() {
    UICommands cmds;

    cmds.setSelectedEntity = [&](EntityId entityId) {
        selectedObject_ = entityId;
    };

    cmds.setBodyPosition =
        [this](float x, float y, float z) {
            ObjectID id = selectedObject_; 
            EditObjectCommand cmd;
            cmd.id      = id;
            cmd.newPose = Pose{ {x,y,z}, bodyState_.orientation, bodyState_.scale };
            cmd.newColour = Colour{ bodyState_.colour.r, bodyState_.colour.g, bodyState_.colour.b };
            cmd.teleport = true;

            worldCmds_.publish(WorldCommand{cmd});
            //std::cout << "UI: set body position to (" << x << "," << y << "," << z << ")\n";
        };

    cmds.setBodyColour = 
        [this](float r, float g, float b) {
        ObjectID id = selectedObject_; 
        EditObjectCommand cmd;
        cmd.id      = id;
        cmd.newPose = Pose{ bodyState_.position, bodyState_.orientation, bodyState_.scale };
        cmd.newColour = Colour{ r,g,b };
        cmd.teleport = true;
        worldCmds_.publish(WorldCommand{cmd});
        
    };

    //Physics property commands
    cmds.setBodyDensity = 
        [this](float density) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.density = density;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyDynamic = 
        [this](bool dynamic) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.dynamic = dynamic;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyLinDamping = 
        [this](float linDamping) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.linDamping = linDamping;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyAngDamping = 
        [this](float angDamping) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.angDamping = angDamping;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyStaticFriction = 
        [this](float staticFriction) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.staticFriction = staticFriction;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyDynamicFriction = 
        [this](float dynamicFriction) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.dynamicFriction = dynamicFriction;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyRestitution = 
        [this](float restitution) {
        ObjectID id = selectedObject_; 
        PatchPhysicsPropsCommand cmd;
        cmd.id = id;
        cmd.patch.restitution = restitution;
        worldCmds_.publish(WorldCommand{cmd});
    };

    cmds.setBodyScale = 
        [this](float scale) { 
        ObjectID id = selectedObject_;
        EditObjectCommand cmd; cmd.id = id;
        cmd.newPose = Pose{ bodyState_.position, bodyState_.orientation, scale };
        cmd.newColour = Colour{ bodyState_.colour.r, bodyState_.colour.g, bodyState_.colour.b };
        cmd.teleport = true;
        worldCmds_.publish(WorldCommand{cmd}); };


        
    // New SphereS
    cmds.createSphere = [&]() {     
    GeometryID sphere = geometryDb_.getByType(SurfaceType::Sphere);
    worldCmds_.publish(WorldCommand{CreateObjectCommand{sphere, Pose{{0.0,5.0,0.0},{0,0,0,1}, 0.2f}, {0.2f,0.2f,0.8f}}});};
    // Camera commands
    cmds.setCameraFov = [&](float fov) { camera_.fovDeg = fov; };
    cmds.setCameraNear = [&](float n) { camera_.znear = n; };
    cmds.setCameraFar  = [&](float f) { camera_.zfar = f; };
    cmds.setCameraAngles = [&](float yaw, float pitch) {
        camera_.yawDeg = yaw;
        camera_.pitchDeg = pitch;
        camera_.clampPitch();
        camera_.updateVectors();
    };

    // ViewportController commands
    cmds.setMoveSpeed        = [&](float v) { viewportCtrl_.setMoveSpeed(v); };
    cmds.setMouseSensitivity = [&](float v) { viewportCtrl_.setMouseSensitivity(v); };
    cmds.setScrollZoomSpeed  = [&](float v) { viewportCtrl_.setScrollZoomSpeed(v); };
    cmds.setInvertY          = [&](bool v)  { viewportCtrl_.setInvertY(v); };
    cmds.setRmbToLook        = [&](bool v)  { viewportCtrl_.setRmbToLook(v); };



    ui_.setCommands(cmds);
}

    

// ========== helpers: primitive builders ==========
static void makeInterleavedPN(const std::vector<glm::vec3>& pos,
                              const std::vector<glm::vec3>& nrm,
                              std::vector<float>& outPN) {
    outPN.resize(pos.size()*6);
    for (size_t i=0;i<pos.size();++i) {
        outPN[i*6+0]=pos[i].x; outPN[i*6+1]=pos[i].y; outPN[i*6+2]=pos[i].z;
        outPN[i*6+3]=nrm[i].x; outPN[i*6+4]=nrm[i].y; outPN[i*6+5]=nrm[i].z;
    }
}

void GlSceneRenderer::createUnitCylinder(MeshGPU& out, int slices = 32) {
    std::vector<glm::vec3> P;
    std::vector<glm::vec3> N;
    std::vector<uint32_t>  I;

    float h = 1.0f;

    // ---- Side surface ----
    for (int i = 0; i <= slices; i++) {
        float u = float(i) / slices;
        float th = u * glm::two_pi<float>();
        float cx = cos(th), sz = sin(th);

        glm::vec3 normal(cx, 0.0f, sz);

        // bottom vertex at y=0
        P.emplace_back(cx, 0.0f, sz);
        N.push_back(normal);

        // top vertex at y=h
        P.emplace_back(cx, h, sz);
        N.push_back(normal);
    }

    // side indices (triangle strip style)
    for (int i = 0; i < slices; i++) {
        uint32_t base = 2 * i;
        I.insert(I.end(), {
            base, base+1, base+2,
            base+1, base+3, base+2
        });
    }

    // ---- Top cap ----
    uint32_t centerTop = (int)P.size();
    P.emplace_back(0, +h, 0);
    N.emplace_back(0, 1, 0);
    for (int i = 0; i <= slices; i++) {
        float th = (float)i / slices * glm::two_pi<float>();
        P.emplace_back(cos(th), +h, sin(th));
        N.emplace_back(0, 1, 0);
    }
    for (int i = 0; i < slices; i++) {
        I.insert(I.end(), { centerTop, centerTop+1+i, centerTop+2+i });
    }

    // ---- Bottom cap ----
    uint32_t centerBot = (int)P.size();
    P.emplace_back(0, 0, 0);
    N.emplace_back(0, -1, 0);
    for (int i = 0; i <= slices; i++) {
        float th = (float)i / slices * glm::two_pi<float>();
        P.emplace_back(cos(th), 0, sin(th));
        N.emplace_back(0, -1, 0);
    }
    for (int i = 0; i < slices; i++) {
        I.insert(I.end(), { centerBot, centerBot+2+i, centerBot+1+i });
    }

    std::vector<float> PN; makeInterleavedPN(P,N,PN);
    out = MeshGPU();
    out.upload(PN, I);
}

void GlSceneRenderer::buildUIState(const WorldSnapshot& snapshot) {
    // ---------- Scene stats ----------

    // ---------- Camera ----------
    camState_.position = camera_.eye;
    camState_.yawDeg   = camera_.yawDeg;
    camState_.pitchDeg = camera_.pitchDeg;
    camState_.fovDeg   = camera_.fovDeg;
    camState_.znear    = camera_.znear;
    camState_.zfar     = camera_.zfar;

    // ---------- Controller ----------
    ctrlState_.moveSpeed        = viewportCtrl_.moveSpeed();
    ctrlState_.mouseSensitivity = viewportCtrl_.mouseSensitivity();
    ctrlState_.scrollZoomSpeed  = viewportCtrl_.scrollZoomSpeed();
    ctrlState_.invertY          = viewportCtrl_.invertY();
    ctrlState_.rmbToLook        = viewportCtrl_.rmbToLook();




    // ---------- Object list ----------
    bodyState_.entityOptions.clear();
    for (const auto& obj : snapshot.objects) {
        bodyState_.entityOptions.push_back(obj.id);
    }

    bodyState_.selectedEntityId =
        (selectedObject_ != 0) ? std::optional<ObjectID>{selectedObject_} : std::nullopt;

    // ---------- Selected object ----------
    if (selectedObject_) {
        auto it = std::find_if(
            snapshot.objects.begin(),
            snapshot.objects.end(),
            [&](const ObjectState& o) { return o.id == selectedObject_; }
        );

        if (it != snapshot.objects.end()) {
            bodyState_.position = it->T_ws.p;
            bodyState_.orientation = it->T_ws.q;
            bodyState_.scale    = it->T_ws.s;
            bodyState_.colour   = it->colourOverride;
            bodyState_.physicsProps = it->physics;
        }
    }
}

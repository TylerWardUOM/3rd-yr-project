#include <memory>
#include "render/GlSceneRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render/shader/MVPUniforms.h"
#include <glm/gtc/matrix_access.hpp>
#include "geometry/GeometryDatabase.h"

// --- utils ---
// build model matrix from Pose
glm::mat4 GlSceneRenderer::compose(const Pose& T) {
    glm::mat4 R = glm::mat4_cast(glm::normalize(glm::quat(T.q)));
    glm::mat4 M = R;
    M[3] = glm::vec4(glm::vec3(T.p), 1.0f);
    return M;
}

// ========== ctor/dtor ==========
GlSceneRenderer::GlSceneRenderer(Window& window, const GeometryDatabase& geomDb, RenderMeshRegistry& meshRegistry)
    : window_(window), 
    shader_("shaders/general.vert", "shaders/general.frag"),
    geometryDb_(geomDb),
    meshRegistry_(meshRegistry),
    camera_(),
    viewportCtrl_(window, camera_),
    imguiLayer_(window_, "#version 330"),
    ui_()

{
    ensurePrimitiveTemplates();
    camera_.eye    = {0.0f, 1.0f, 1.0f};
    camera_.up     = {0.0f, 1.0f, 0.0f};
    camera_.fovDeg = 60.f;
    camera_.yawDeg   = 0.f;
    camera_.pitchDeg = -45.f;
    camera_.updateVectors();


         // Initial viewport size
    int fbw=0, fbh=0; 
    window_.getFramebufferSize(fbw, fbh);
    if (fbw>0 && fbh>0) {
        camera_.aspect = float(fbw)/float(fbh); 
    }
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


void GlSceneRenderer::render(const WorldSnapshot& snapshot) {
    // basic Gl state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    for (const ObjectState& obj : snapshot.objects) {

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

        // --- Render ---
        r.render(camera_, compose(obj.T_ws));
    }

    viewportCtrl_.update(0.0f, false); // no dt, no UI capture

;
    imguiLayer_.begin();
    buildUIState(snapshot);
    // ui_.drawBodyPanel(bodyState_);
    imguiLayer_.getFps(stats_.fps);
    ui_.drawDebugPanel(stats_);
    ui_.drawCameraPanel(camState_);
    ui_.drawControllerPanel(ctrlState_);
    imguiLayer_.end();
    window_.swap();
    window_.poll();
    
    // ---- draw surfaces/links with basic shader ----


    // // draw surfaces
    // for (uint32_t i=0; i<world_.numSurfaces; ++i) {
    //     const auto& s = world_.surfaces[i];
    //     glm::mat4 M = compose(s.T_ws);

    //     switch (s.type) {
    //     case SurfaceType::Plane: {
    //         glm::vec2 half = {25.f,25.f};
    //         drawPlaneRenderable(M, half, s.colour);
    //     } break;

    //     case SurfaceType::Sphere:
    //         drawSphereRenderable(M, s.sphere.radius, s.colour);
    //         break;

    //     case SurfaceType::TriMesh: {
    //         auto it = triMeshes_.find(s.mesh);
    //         if (it != triMeshes_.end()) drawMeshRenderable(it->second, M, s.colour);
    //     } break;
    //     }
    // }
    // drawOverlays();
}

inline glm::dvec3 getTranslation(const glm::mat4& M) {
    return glm::vec3(M[3]); // xyz of 4th column
}

// void GlSceneRenderer::drawOverlays() {
//     //Maybe change to use the angles sent from the haptic device instead of FK
    
//     // pick small radii for ghosts
//     drawSphereRenderable(compose(haptic_.devicePose_ws), 0.02f, {0.1f,1.0f,0.1f}); // device 
//     drawSphereRenderable(compose(haptic_.refPose_ws),    0.018f, {1.0f,0.1f,0.1f}); // ref   
//     drawSphereRenderable(compose(haptic_.proxyPose_ws),  0.022f, {0.1f,0.1f,1.0f}); // proxy 

//     basicRobot robot = {0.15, 0.15};
//     float jointAngles[2];
//     jointAngles[0] = haptic_.latestAngles[0];
//     jointAngles[1] = haptic_.latestAngles[1];

//     glm::dvec3 p0(0,0,0); // shoulder at origin
//     glm::dvec3 p1; // elbow position
//     glm::dvec3 p2; // end-effector position

//     auto Ts = forwardExplicitAll(robot.link1, robot.link2, glm::dvec3(0.0, jointAngles[0], jointAngles[1]));
//     p1 = getTranslation(Ts[2]); // elbow
//     p2 = getTranslation(Ts[3]); // end-effector
//     // draw links
//     Pose p_l1 = linkPoseBetween(p0, p1);
//     Pose p_l2 = linkPoseBetween(p1, p2);
//     drawCylinderRenderable(compose(p_l1), 0.01, glm::length(p1 - p0), {0.8f,0.8f,0.8f}); // link 1
//     drawCylinderRenderable(compose(p_l2), 0.01, glm::length(p2 - p1), {0.8f,0.8f,0.8f}); // link 2


//     drawSphereRenderable(compose(Pose{p0, {0,0,0,1}}), 0.03, {1.0f,1.0f,0.1f}); // joint 0
//     drawSphereRenderable(compose(Pose{p1, {0,0,0,1}}), 0.03, {1.0f,1.0f,0.1f}); // joint 1
//     drawSphereRenderable(compose(Pose{p2, {0,0,0,1}}), 0.03, {1.0f,1.0f,0.1f}); // end effector
// }



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

void GlSceneRenderer::createUnitPlane(MeshGPU& out) {
    // XZ quad at y=0, size 1x1, centered on origin
    std::vector<glm::vec3> P = {
        {-0.5f, 0.f, -0.5f}, { 0.5f, 0.f, -0.5f},
        { 0.5f, 0.f,  0.5f}, {-0.5f, 0.f,  0.5f}
    };
    std::vector<glm::vec3> N(4, {0.f,1.f,0.f});
    std::vector<uint32_t>   I = {0,1,2, 0,2,3};

    std::vector<float> PN; makeInterleavedPN(P,N,PN);
    out = MeshGPU();
    out.upload(PN, I);

}

void GlSceneRenderer::createUnitSphere(MeshGPU& out) {
    // quick-and-dirty icosphere (or latitude-longitude); here’s a tiny lat-long sphere
    const int stacks = 16, slices = 24;
    std::vector<glm::vec3> P;
    std::vector<glm::vec3> N;
    std::vector<uint32_t>  I;

    for (int i=0;i<=stacks;i++){
        float v  = float(i)/stacks;
        float phi = v*glm::pi<float>();
        for (int j=0;j<=slices;j++){
            float u  = float(j)/slices;
            float th = u*glm::two_pi<float>();
            glm::vec3 n = { std::sin(phi)*std::cos(th),
                            std::cos(phi),
                            std::sin(phi)*std::sin(th) };
            P.push_back(n); // radius = 1
            N.push_back(glm::normalize(n));
        }
    }
    auto idx = [slices](int i,int j){ return i*(slices+1)+j; };
    for (int i=0;i<stacks;i++){
        for (int j=0;j<slices;j++){
            uint32_t a=idx(i,j), b=idx(i+1,j), c=idx(i+1,j+1), d=idx(i,j+1);
            I.insert(I.end(), {a,b,c, a,c,d});
        }
    }

    std::vector<float> PN; makeInterleavedPN(P,N,PN);
    out = MeshGPU();
    out.upload(PN, I);
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

    // // ---------- Object list ----------
    // bodyState_.entityOptions.clear();
    // for (const auto& obj : snapshot.objects) {
    //     bodyState_.entityOptions.push_back(obj.id);
    // }

    // bodyState_.selectedEntityId =
    //     (selected_ != 0) ? std::optional<ObjectID>{selected_} : std::nullopt;

    // // ---------- Selected object ----------
    // if (selected_) {
    //     auto it = std::find_if(
    //         snapshot.objects.begin(),
    //         snapshot.objects.end(),
    //         [&](const ObjectState& o) { return o.id == selected_; }
    //     );

    //     if (it != snapshot.objects.end()) {
    //         bodyState_.position = it->T_ws.p;
    //         bodyState_.colour   = it->colourOverride;
    //         bodyState_.physicsProps = {/* fill if available */};
    //     }
    // }
}

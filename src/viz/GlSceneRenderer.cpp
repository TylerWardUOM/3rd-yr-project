#include <glad/glad.h>                  // Gl first
#include <memory>
#include "viz/GlSceneRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "viz/MVPUniforms.h"

// --- utils ---
// build model matrix from Pose
glm::mat4 GlSceneRenderer::compose(const Pose& T) {
    glm::mat4 R = glm::mat4_cast(glm::normalize(glm::quat(T.q)));
    glm::mat4 M = R;
    M[3] = glm::vec4(glm::vec3(T.p), 1.0f);
    return M;
}

// ========== ctor/dtor ==========
GlSceneRenderer::GlSceneRenderer(Camera& cam)
    : camera_(cam),
    shader_("shaders/general.vert", "shaders/basic.frag")
{
    ensurePrimitiveTemplates();
}

GlSceneRenderer::~GlSceneRenderer() {}

// ========== ISceneRenderer impl ==========
void GlSceneRenderer::ensurePrimitiveTemplates() {
    if (unitSphere_.isUploaded() == 0) createUnitSphere(unitSphere_);
    if (unitPlane_.isUploaded()  == 0) createUnitPlane(unitPlane_);
}

void GlSceneRenderer::registerTriMesh(MeshId id,
                                      const float* pn, size_t pnCount,
                                      const uint32_t* idx, size_t idxCount)
{
    auto [it, inserted] = triMeshes_.try_emplace(id);  // default-construct MeshGPU if new
    it->second.upload(std::vector<float>(pn, pn + pnCount), std::vector<uint32_t>(idx, idx + idxCount)); // maybe replace tempoary vectors with pointers in MeshGPU::upload
}

void GlSceneRenderer::onResize(int width, int height) {
    fbw_ = width; fbh_ = height;
    glViewport(0, 0, fbw_, fbh_);
}

void GlSceneRenderer::setViewProj(const glm::mat4& V, const glm::mat4& P) {
    V_ = V; P_ = P;
}

void GlSceneRenderer::submit(const WorldSnapshot& world,
                             const HapticsVizSnapshot& viz) {
    world_ = world; // POD copy
    viz_   = viz;
}

void GlSceneRenderer::render() {
    // basic Gl state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- draw surfaces/links with basic shader ----


    // draw surfaces
    for (uint32_t i=0; i<world_.numSurfaces; ++i) {
        const auto& s = world_.surfaces[i];
        glm::mat4 M = compose(s.T_ws);

        switch (s.type) {
        case SurfaceType::Plane: {
            glm::vec2 half = {25.f,25.f};
            drawPlaneRenderable(M, half, s.colour);
        } break;

        case SurfaceType::Sphere:
            drawSphereRenderable(M, s.sphere.radius, s.colour);
            break;

        case SurfaceType::TriMesh: {
            auto it = triMeshes_.find(s.mesh);
            if (it != triMeshes_.end()) drawMeshRenderable(it->second, M, s.colour);
        } break;
        }
    }
}

// ========== helpers: draw ==========
void GlSceneRenderer::drawMeshRenderable(const MeshGPU& m, const glm::mat4& M, const glm::vec3& colour) {
    Renderable r;
    r.mesh = &m;
    r.shader = &shader_;
    r.colour = colour;
    r.render(camera_, M);
}

void GlSceneRenderer::drawPlaneRenderable(const glm::mat4& M, const glm::vec2& halfExtents, const glm::vec3& colour) {
    glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(halfExtents.x*2.f, 1.f, halfExtents.y*2.f));
    Renderable r;
    r.mesh = &unitPlane_;  
    r.shader = &shader_;
    r.colour = colour;
    r.render(camera_, M * S);
}


void GlSceneRenderer::drawSphereRenderable(const glm::mat4& M, double radius, const glm::vec3& colour) {
    glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3((float)radius));
    Renderable r;
    r.mesh = &unitSphere_; 
    r.shader = &shader_;
    r.colour = colour;
    r.render(camera_, M * S);}



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
            P.push_back(n * 0.5f); // radius = 0.5
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

#include "render/RenderMeshRegistry.h"
#include <vector>
#include <glm/vec3.hpp>
#include <cmath>
#include <glm/gtc/quaternion.hpp>  // glm::quat, normalize(quat)
#include <iostream>

// --- Forward mesh builders ---
static MeshGPU makePlaneMesh();
static MeshGPU makeSphereMesh();

RenderMeshHandle RenderMeshRegistry::getOrCreate(MeshKind kind) {
    auto it = kindToHandle_.find(kind);
    if (it != kindToHandle_.end()) {
        return it->second;
    }
    std::cout << "RenderMeshRegistry: creating mesh for kind " << static_cast<int>(kind) << std::endl;
    RenderMeshHandle h = createMesh(kind);
    kindToHandle_[kind] = h;
    return h;
}

const MeshGPU* RenderMeshRegistry::get(RenderMeshHandle handle) const {
    auto it = meshes_.find(handle);
    return (it == meshes_.end()) ? nullptr : &it->second;
}

RenderMeshHandle RenderMeshRegistry::createMesh(MeshKind kind) {
    MeshGPU mesh;

    switch (kind) {
    case MeshKind::Plane:
        mesh = makePlaneMesh();
        break;
    case MeshKind::Sphere:
        mesh = makeSphereMesh();
        break;
    default:
        return 0;
    }

    RenderMeshHandle handle = nextHandle_++;
    meshes_.emplace(handle, std::move(mesh));
    return handle;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static void makeInterleavedPN(const std::vector<glm::vec3>& pos,
                              const std::vector<glm::vec3>& nrm,
                              std::vector<float>& outPN) {
    outPN.clear();
    outPN.reserve(pos.size() * 6);

    for (size_t i = 0; i < pos.size(); ++i) {
        outPN.push_back(pos[i].x);
        outPN.push_back(pos[i].y);
        outPN.push_back(pos[i].z);
        outPN.push_back(nrm[i].x);
        outPN.push_back(nrm[i].y);
        outPN.push_back(nrm[i].z);
    }
}

// ------------------------------------------------------------
// Plane mesh (XZ quad at y=0)
// ------------------------------------------------------------

static MeshGPU makePlaneMesh() {
    MeshGPU mesh;

    std::vector<glm::vec3> P = {
        {-0.5f, 0.f, -0.5f}, { 0.5f, 0.f, -0.5f},
        { 0.5f, 0.f,  0.5f}, {-0.5f, 0.f,  0.5f}
    };

    std::vector<glm::vec3> N(4, {0.f, 1.f, 0.f});
    std::vector<uint32_t>  I = {0,1,2, 0,2,3};

    std::vector<float> PN;
    makeInterleavedPN(P, N, PN);

    mesh.upload(PN, I);
    return mesh;
}

// ------------------------------------------------------------
// Sphere mesh (very coarse debug sphere)
// ------------------------------------------------------------

static MeshGPU makeSphereMesh() {
    MeshGPU mesh;
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
    mesh.upload(PN, I);
    return mesh;
}

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

    // Octahedron-based sphere (simple & cheap)
    std::vector<glm::vec3> P = {
        { 0, 1, 0},
        { 1, 0, 0},
        { 0, 0, 1},
        {-1, 0, 0},
        { 0, 0,-1},
        { 0,-1, 0}
    };

    std::vector<glm::vec3> N;
    N.reserve(P.size());
    for (const auto& p : P) {
        N.push_back(glm::normalize(p));
    }

    std::vector<uint32_t> I = {
        0,1,2, 0,2,3, 0,3,4, 0,4,1,
        5,2,1, 5,3,2, 5,4,3, 5,1,4
    };

    std::vector<float> PN;
    makeInterleavedPN(P, N, PN);

    mesh.upload(PN, I);
    return mesh;
}

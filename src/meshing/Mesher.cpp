#include "meshing/Mesher.h"
#include <cmath>

Mesh Mesher::makeMeshMC(const EnvInterface& env,
                        const glm::dvec3& minB, const glm::dvec3& maxB,
                        int nx, int ny, int nz, double iso)
{
    MarchingCubes mc;
    return mc.generateMeshFromSDF(
        [&](const glm::dvec3& p){ return env.phi(p); },
        minB, maxB, nx, ny, nz, iso,
        [&](const glm::dvec3& p){ return env.grad(p); }
    );
}

void Mesher::packPosNrmIdx(const Mesh& m,
                           std::vector<float>& outPN,
                           std::vector<unsigned>& outIdx) {
    outPN.clear(); outIdx.clear();
    outPN.reserve(m.vertices.size() * 6);
    outIdx.reserve(m.indices.size());

    for (const auto& v : m.vertices) {
        outPN.push_back((float)v.pos.x);
        outPN.push_back((float)v.pos.y);
        outPN.push_back((float)v.pos.z);

        const double L = std::sqrt(v.nrm.x*v.nrm.x + v.nrm.y*v.nrm.y + v.nrm.z*v.nrm.z);
        const double inv = (L > 1e-12) ? 1.0/L : 1.0;
        outPN.push_back((float)(v.nrm.x * inv));
        outPN.push_back((float)(v.nrm.y * inv));
        outPN.push_back((float)(v.nrm.z * inv));
    }

    for (auto idx : m.indices)
        outIdx.push_back(static_cast<unsigned>(idx));
}


// src/env/meshing/Mesher.cpp
#include "env/Mesher.h"
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

// Pack mesh vertices as float array: [px,py,pz,nx,ny,nz]...
std::vector<float> Mesher::packPosNrmFloat(const Mesh& m) {
    std::vector<float> out; out.reserve(m.vertices.size()*6);
    for (const auto& v : m.vertices) {
        out.push_back((float)v.pos.x); out.push_back((float)v.pos.y); out.push_back((float)v.pos.z);
        const double L = std::sqrt(v.nrm.x*v.nrm.x + v.nrm.y*v.nrm.y + v.nrm.z*v.nrm.z);
        const double inv = (L>1e-12)? 1.0/L : 1.0;
        out.push_back((float)(v.nrm.x*inv)); out.push_back((float)(v.nrm.y*inv)); out.push_back((float)(v.nrm.z*inv));
    }
    return out;
}

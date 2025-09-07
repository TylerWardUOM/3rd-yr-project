#include "meshing/MarchingCubes.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>              // glm::dvec3, dot/cross/normalize
#include "meshing/MarchingCubesTables.h"
using namespace mc_tables;

// ---------- Private static helpers ----------

std::size_t MarchingCubes::gridIndex(int i, int j, int k, int ny, int nz) {
    return static_cast<std::size_t>((i * ny + j) * nz + k);
}

std::vector<double> MarchingCubes::sampleSDFGrid(
    const std::function<double(const glm::dvec3&)>& sdf,
    const glm::dvec3& minB, const glm::dvec3& step,
    int nx, int ny, int nz)
{
    std::vector<double> F(static_cast<std::size_t>(nx)*ny*nz);
    for (int i=0;i<nx;i++){
        for (int j=0;j<ny;j++){
            for (int k=0;k<nz;k++){
                glm::dvec3 p{
                    minB.x + i*step.x,
                    minB.y + j*step.y,
                    minB.z + k*step.z
                };
                F[gridIndex(i,j,k,ny,nz)] = sdf(p);
            }
        }
    }
    return F;
}


glm::dvec3 MarchingCubes::lerpEdge(
    const glm::dvec3& p0, const glm::dvec3& p1,
    double f0, double f1, double iso)
{
    const double denom = (f1 - f0);
    double t = (std::fabs(denom) > 1e-12) ? (iso - f0)/denom : 0.5;
    if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
    return p0*(1.0 - t) + p1*t;
}

int MarchingCubes::computeCubeIndex(const double S[8], double iso) {
    int idx = 0;
    for (int c=0;c<8;c++) if (S[c] < iso) idx |= (1<<c);
    return idx;
}

void MarchingCubes::appendOrientedTri(
    Mesh& mesh,
    const glm::dvec3& p0, const glm::dvec3& n0,
    const glm::dvec3& p1, const glm::dvec3& n1,
    const glm::dvec3& p2, const glm::dvec3& n2)
{
    const uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.push_back({p0,n0});
    mesh.vertices.push_back({p1,n1});
    mesh.vertices.push_back({p2,n2});

    const glm::dvec3 triN = glm::normalize(glm::cross(p1 - p0, p2 - p0));
    const glm::dvec3 avgN = glm::normalize((n0 + n1 + n2) / 3.0);
    const bool flip = (glm::dot(triN, avgN) < 0.0);

    if (!flip) {
        mesh.indices.push_back(base+0);
        mesh.indices.push_back(base+1);
        mesh.indices.push_back(base+2);
    } else {
        mesh.indices.push_back(base+0);
        mesh.indices.push_back(base+2);
        mesh.indices.push_back(base+1);
    }
}

// ---------- Public API ----------

Mesh MarchingCubes::generateMeshFromSDF(
    const std::function<double(const glm::dvec3&)>& sdf,
    const glm::dvec3& minBound, const glm::dvec3& maxBound,
    int nx, int ny, int nz, double iso,
    const std::function<glm::dvec3(const glm::dvec3&)>& gradFn) const
{
    Mesh mesh;
    if (nx < 2 || ny < 2 || nz < 2) return mesh;

    // Grid spacing & finite-difference step for normals
    const glm::dvec3 size = maxBound - minBound;
    const glm::dvec3 step = {
        size.x / double(nx-1),
        size.y / double(ny-1),
        size.z / double(nz-1)
    };

    // Sample scalar field
    std::vector<double> F = sampleSDFGrid(sdf, minBound, step, nx, ny, nz);

    // Heuristic reserves
    mesh.vertices.reserve((nx-1)*(ny-1)*(nz-1)*3/2);
    mesh.indices .reserve((nx-1)*(ny-1)*(nz-1)*3);

    // March voxels
    for (int i=0;i<nx-1;i++){
        for (int j=0;j<ny-1;j++){
            for (int k=0;k<nz-1;k++){
                // Gather corners
                glm::dvec3 P[8]; double S[8];
                for (int c=0;c<8;c++){
                    const int ci = i + CORNER[c][0];
                    const int cj = j + CORNER[c][1];
                    const int ck = k + CORNER[c][2];
                    P[c] = {
                        minBound.x + ci*step.x,
                        minBound.y + cj*step.y,
                        minBound.z + ck*step.z
                    };
                    S[c] = F[gridIndex(ci,cj,ck,ny,nz)];
                }

                const int caseIdx = computeCubeIndex(S, iso);
                const int eMask   = edgeTable[caseIdx];
                if (eMask == 0) continue;

                // Interpolate edge vertices used in this case
                glm::dvec3 E[12];
                for (int e=0;e<12;e++){
                    if (eMask & (1<<e)) {
                        const int c0 = EDGE2C[e][0], c1 = EDGE2C[e][1];
                        E[e] = lerpEdge(P[c0], P[c1], S[c0], S[c1], iso);
                    }
                }

                // Emit triangles
                for (int t=0; triTable[caseIdx][t] != -1; t += 3){
                    const int e0 = triTable[caseIdx][t+0];
                    const int e1 = triTable[caseIdx][t+1];
                    const int e2 = triTable[caseIdx][t+2];

                    const glm::dvec3 p0 = E[e0];
                    const glm::dvec3 p1 = E[e1];
                    const glm::dvec3 p2 = E[e2];

                    const glm::dvec3 n0 = gradFn(p0);
                    const glm::dvec3 n1 = gradFn(p1);
                    const glm::dvec3 n2 = gradFn(p2);

                    appendOrientedTri(mesh, p0,n0, p1,n1, p2,n2);
                }
            }
        }
    }

    return mesh;
}

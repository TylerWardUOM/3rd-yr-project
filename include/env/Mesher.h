// include/env/Mesher.h
#pragma once
#include <vector>                
#include <glm/glm.hpp>
#include "env/env_interface.h"
#include "env/MarchingCubes.h"

class Mesher {
public:
    static Mesh makeMeshMC(
        const EnvInterface& env,
        const glm::dvec3& minB, const glm::dvec3& maxB,
        int nx, int ny, int nz, double iso = 0.0);

    static void packPosNrmIdx(
        const Mesh& m,
        std::vector<float>& outPN, // interleaved pos+nrm
        std::vector<unsigned>& outIdx);
};

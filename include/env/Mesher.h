// include/env/Mesher.h
#pragma once
#include "env/env_interface.h"
#include "env/MarchingCubes.h"
#include <glm/glm.hpp>

class Mesher {
public:
    static Mesh makeMeshMC(
        const EnvInterface& env,
        const glm::dvec3& minB, const glm::dvec3& maxB,
        int nx, int ny, int nz, double iso = 0.0);

    static std::vector<float> packPosNrmFloat(const Mesh& m);
};

#pragma once
#include "Pose.h"
#include "SurfaceDef.h"
#include <cstdint>

struct LinkPose { EntityId id; Pose T_ws; };

struct WorldSnapshot {
    double   t_sec = 0.0; // timestamp of this snapshot
    // Surfaces (planes/spheres/trimesh instances) that haptics/render need
    SurfaceDef  surfaces[256];  
    uint32_t numSurfaces = 0;

    // Add Robot Links Later and their poses

};

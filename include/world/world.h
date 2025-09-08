#pragma once
#include <memory>
#include <vector>
#include <atomic>
#include <glm/glm.hpp>
#include "viz/Renderable.h"
#include "env/env_interface.h"


struct MCParams {
    glm::dvec3 minB{-1.5,-1.5,-1.5};
    glm::dvec3 maxB{ 1.5, 1.5, 1.5};
    int nx{64}, ny{64}, nz{64};
    double iso{0.0};
};


using BodyId = uint32_t; 
static constexpr BodyId kInvalidBody = UINT32_MAX; 

// Simple pose (position + orientation)
struct Pose { 
    glm::dvec3 p{0.0,0.0,0.0}; // Position translation
    glm::dquat q{1.0, 0.0, 0.0, 0.0};  // Orientation quaternion rotation
};

// Stores a body’s static env primative and other immutable data
struct BodyStatic {
    std::shared_ptr<const EnvInterface> env; 
};

// A buffer of poses with a sequence number for lock-free reading/writing
struct PoseBuffer {
    std::atomic<uint64_t> seq{0};
    std::vector<Pose> poses;
};

// Two PoseBuffers for double-buffering
struct Poses {
    PoseBuffer buf[2];
    std::atomic<uint8_t> read_index{0};
};

// The main world structure containing bodies, poses, and renderables
struct World {
    std::vector<BodyStatic> statics;
    Poses poses;
    std::vector<Renderable> renderables;
};

BodyId addBody(World& W, std::shared_ptr<const EnvInterface> env, Shader* shader, const MCParams& mc);
void writePose(World& W, BodyId id, const Pose& p);
bool readPose(const World& W, BodyId id, Pose& out);
#pragma once
#include "world/WorldSnapshot.h"
#include "world/Pose.h"
struct HapticsVizSnapshot { Pose refTool{}, proxyTool{}; };

class ISceneRenderer {
public:
    virtual ~ISceneRenderer() = default;
    virtual void ensurePrimitiveTemplates() = 0; // unit sphere/plane
    virtual void registerTriMesh(MeshId id,
                                const float* posNorm, size_t pnCount,
                                const uint32_t* idx, size_t idxCount) = 0;
    virtual void onResize(int width, int height) = 0;  
    virtual void setViewProj(const glm::mat4& V, const glm::mat4& P) = 0;
    virtual void submit(const WorldSnapshot& world, const HapticsVizSnapshot& viz) = 0; // submit for rendering
    virtual void render() = 0; // render submitted scene
};

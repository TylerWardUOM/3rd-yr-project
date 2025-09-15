#pragma once
#include "world/WorldSnapshot.h"
#include "haptics/HapticBuffers.h"
#include "world/Pose.h"

/// @ingroup scene
/// @brief Scene renderer interface: defines methods for rendering the scene
class ISceneRenderer {
public:
    /// @brief Destructor
    virtual ~ISceneRenderer() = default;

    /// @brief Ensure primitive templates are initialized (e.g. unit sphere/plane)
    virtual void ensurePrimitiveTemplates() = 0; // unit sphere/plane
    
    /// @brief Register a triangle mesh for rendering
    /// @param id Mesh ID
    /// @param posNorm Pointer to array of vertex positions and normals (interleaved)
    /// @param pnCount Number of position/normal pairs
    /// @param idx Pointer to array of triangle indices
    /// @param idxCount Number of indices
    virtual void registerTriMesh(MeshId id,
                                const float* posNorm, size_t pnCount,
                                const uint32_t* idx, size_t idxCount) = 0;
    /// @brief Handle window resize event
    /// @param width 
    /// @param height 
    virtual void onResize(int width, int height) = 0; 
    
    /// @brief Set the view and projection matrices for rendering 
    /// @param V View matrix
    /// @param P Projection matrix
    virtual void setViewProj(const glm::mat4& V, const glm::mat4& P) = 0;

    /// @brief Submit the current world and haptic snapshots for rendering
    /// @param world Current world snapshot
    /// @param haptic Current haptic snapshot   
    virtual void submit(const WorldSnapshot& world, const HapticSnapshot& haptic) = 0; // submit for rendering

    /// @brief Render the submitted scene
    virtual void render() = 0; // render submitted scene
};

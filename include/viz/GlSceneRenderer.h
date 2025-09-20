#ifndef GLSCENERENDERER_H
#define GLSCENERENDERER_H

#include <unordered_map>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene/ISceneRenderer.h"
#include "viz/Shader.h"
#include "viz/MeshGPU.h"
#include "viz/Renderable.h"
#include "viz/Camera.h"


class GlSceneRenderer : public ISceneRenderer {
    public:

        GlSceneRenderer(Camera& cam);
        ~GlSceneRenderer() override;

        // --- ISceneRenderer interface ---
        void ensurePrimitiveTemplates() override; // unit sphere/plane
        void registerTriMesh(MeshId id,
                            const float* posNorm, size_t pnCount,
                            const uint32_t* idx, size_t idxCount) override;
        void onResize(int width, int height) override;
        void setViewProj(const glm::mat4& V, const glm::mat4& P) override;
        void submit(const WorldSnapshot& world, const HapticSnapshot& haptic) override; // submit for rendering
        void render() override; // render submitted scene

    private:
        Camera& camera_;
        // --- cached last-submitted state
        WorldSnapshot       world_{};
        HapticSnapshot  haptic_{};

        // --- GPU resources
        Shader shader_;   // generic lit shader (pos+norm)

        MeshGPU unitSphere_;
        MeshGPU unitPlane_;
        MeshGPU unitCylinder_;

        std::unordered_map<MeshId, MeshGPU> triMeshes_; // custom meshes

        // --- per-frame state
        glm::mat4 V_{1.0f}, P_{1.0f}; // view/proj matrices
        int fbw_ = 0, fbh_ = 0; // framebuffer size (for aspect)

        // --- helpers

        static glm::mat4 compose(const Pose& T);
        static void createUnitSphere(MeshGPU& out); // icosphere
        static void createUnitPlane(MeshGPU& out);  // quad on XZ at y=0
        static void createUnitCylinder(MeshGPU& out, int slices); // along Y axis, height 1, radius 1

        void drawMeshRenderable(const MeshGPU& m, const glm::mat4& M, const glm::vec3& colour);
        void drawPlaneRenderable(const glm::mat4& M, const glm::vec2& halfExtents, const glm::vec3& colour);
        void drawSphereRenderable(const glm::mat4& M, double radius, const glm::vec3& colour);
        void drawCylinderRenderable(const glm::mat4& M, double radius, double height, const glm::vec3& colour);

        void drawOverlays();

};

#endif // GLSCENERENDERER_H
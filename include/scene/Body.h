#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Env / meshing forward decls (minimize header coupling)
class EnvInterface;

// Viz & scene types used *by value* need full headers
#include "meshing/Mesher.h"
#include "viz/Renderable.h"

class Body {
public:
    Body(std::unique_ptr<EnvInterface> primitive,
         std::unique_ptr<MeshGPU>      mesh,
         Mesher*                        mesher,
         Shader*                        shader);

    // ---------- Transform control ----------
    Transform&       transform();
    const Transform& transform() const;

    void setModel(const glm::mat4& M);
    void translate(const glm::vec3& dt);
    void rotate(float radians, const glm::vec3& axis);
    void setRotation(float radians, const glm::vec3& axis);
    void setPosition(const glm::vec3& pos);

    glm::vec3 getPosition() const;

    // ---------- Meshing ----------
    void remeshIfPossible(); // build triangles from SDF & upload to GPU

    // ---------- Rendering ----------
    void render(const Camera& cam);

    // ---------- Accessors ----------
    EnvInterface*       primitive();
    const EnvInterface* primitive() const;
    MeshGPU*            mesh();

private:
    void syncSDF(); // keep SDF’s cached WORLD params in sync with model matrix

private:
    // World state
    Transform transform_{};

    // Core composition
    std::unique_ptr<EnvInterface> prim_;
    std::unique_ptr<MeshGPU>      mesh_;
    Mesher*                       mesher_{nullptr}; // non-owning
    Renderable                    renderable_{};    // draws mesh_ with transform_ and shader
};

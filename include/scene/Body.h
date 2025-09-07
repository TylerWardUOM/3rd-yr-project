#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Env / meshing forward decls (minimize header coupling)
class EnvInterface;


// Viz & scene types used *by value* need full headers
#include "meshing/Mesher.h"
#include "viz/Renderable.h"


struct MCParams {
    glm::dvec3 minB{-1.5,-1.5,-1.5};
    glm::dvec3 maxB{ 1.5, 1.5, 1.5};
    int nx{64}, ny{64}, nz{64};
    double iso{0.0};
};

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

    void setMCBounds(const glm::dvec3& minB, const glm::dvec3& maxB) { mc_.minB=minB; mc_.maxB=maxB; }
    void setMCRes(int nx,int ny,int nz){ mc_.nx=nx; mc_.ny=ny; mc_.nz=nz; }

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

    MCParams mc_;
    // World state
    Transform transform_{};

    // Core composition
    std::unique_ptr<EnvInterface> prim_;
    std::unique_ptr<MeshGPU>      mesh_;
    Mesher*                       mesher_{nullptr}; // non-owning
    Renderable                    renderable_{};    // draws mesh_ with transform_ and shader
};

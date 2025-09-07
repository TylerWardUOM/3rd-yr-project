#include "scene/Body.h"

#include <glm/gtc/matrix_transform.hpp>
#include "env/env_interface.h"
#include "meshing/Mesher.h"


// ---------- Ctor ----------
Body::Body(std::unique_ptr<EnvInterface> primitive,
           std::unique_ptr<MeshGPU>      mesh,
           Mesher*                        mesher,
           Shader*                        shader)
    : prim_(std::move(primitive)),
      mesh_(std::move(mesh)),
      mesher_(mesher)
{
    // Wire our internal Renderable to the owned mesh + transform + provided shader
    renderable_.mesh      = mesh_.get();
    renderable_.shader    = shader;
    renderable_.transform = transform_; // value copy; kept in sync before render

    // Ensure SDF is in sync with model once at construction
    remeshIfPossible();
    syncSDF();
}

// ---------- Transform control ----------
Transform& Body::transform() { return transform_; }
const Transform& Body::transform() const { return transform_; }

void Body::setModel(const glm::mat4& M) {
    transform_.model = M;
    syncSDF();
}

void Body::translate(const glm::vec3& dt) {
    transform_.model = glm::translate(transform_.model, dt);
    syncSDF();
}

void Body::rotate(float radians, const glm::vec3& axis) {
    transform_.model = glm::rotate(transform_.model, radians, axis);
    syncSDF();
}

// ---------- Meshing ----------
void Body::remeshIfPossible() {
    if (!mesher_ || !prim_ || !mesh_) return;

    // Build CPU mesh from SDF in WORLD space.
    // Adjust bounds/resolution/iso as needed 
    Mesh m = Mesher::makeMeshMC(
        *prim_,
        glm::dvec3(-1.5, -1.5, -1.5), glm::dvec3(1.5, 1.5, 1.5), // bounds
        64, 64, 64,                                              // resolution
        0.0                                                      // iso
    );

    std::vector<float>    interleavedPN;
    std::vector<unsigned> indices;
    Mesher::packPosNrmIdx(m, interleavedPN, indices);

    mesh_->upload(interleavedPN, indices); // GPU upload
}

// ---------- Rendering ----------
void Body::render(const Camera& cam) {
    // Keep Renderable’s transform in sync right before drawing
    renderable_.transform = transform_;
    renderable_.render(cam);
}

// ---------- Accessors ----------
EnvInterface* Body::primitive() { return prim_.get(); }
const EnvInterface* Body::primitive() const { return prim_.get(); }
MeshGPU* Body::mesh() { return mesh_.get(); }

// ---------- Private ----------
void Body::syncSDF() {
    if (prim_) prim_->update(transform_.model);
}

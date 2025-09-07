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

void Body::setRotation(float radians, const glm::vec3& axis) {
    // Extract current translation
    glm::vec3 translation = glm::vec3(transform_.model[3]);

    // Start from identity and apply translation + new rotation
    transform_.model = glm::mat4(1.0f);
    transform_.model = glm::translate(transform_.model, translation);
    transform_.model = glm::rotate(transform_.model, radians, axis);

    syncSDF();
}

glm::vec3 Body::getPosition() const {
    // Extract translation directly from model matrix
    return glm::vec3(transform_.model[3]);
}


void Body::setPosition(const glm::vec3& pos) {
    // Keep the current rotation + scale
    glm::mat4 model = transform_.model;
    model[3] = glm::vec4(pos, 1.0f);  // overwrite translation column

    transform_.model = model;
    syncSDF();
}

// ---------- Meshing ----------
void Body::remeshIfPossible() {
    if (!mesher_ || !prim_ || !mesh_) return;

    // Build CPU mesh from SDF in WORLD space.
    // Adjust bounds/resolution/iso as needed 
    Mesh m = Mesher::makeMeshMC(*prim_, mc_.minB, mc_.maxB, mc_.nx, mc_.ny, mc_.nz, mc_.iso);


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

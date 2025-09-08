#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

struct Ray { glm::vec3 o; glm::vec3 d; }; // origin + unit dir

// Create a world-space ray from NDC coords and inverse VP matrix
inline Ray rayFromNDC(const glm::vec2& ndc, const glm::mat4& invVP) {
    // points on near/far planes in NDC
    glm::vec4 np = invVP * glm::vec4(ndc.x, ndc.y, -1.f, 1.f); // near plane
    glm::vec4 fp = invVP * glm::vec4(ndc.x, ndc.y,  1.f, 1.f); // far plane
    np /= np.w; fp /= fp.w; // perspective divide
    glm::vec3 o = glm::vec3(np);
    glm::vec3 d = glm::normalize(glm::vec3(fp - np));
    return {o,d}; // return ray
}

inline bool intersectRayPlaneY0(const Ray& r, float& t, glm::vec3& hit) {
    // Plane: y = 0, normal (0,1,0)
    const float denom = r.d.y;
    if (fabsf(denom) < 1e-6f) return false; // parallel
    t = -r.o.y / denom;
    if (t < 0.f) return false; // behind camera
    hit = r.o + t * r.d;
    return true;
}

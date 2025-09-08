#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "viz/Camera.h"
#include "viz/ray.h"


// Convert window pixel coords to NDC (-1 to +1)
inline glm::vec2 toNDC(double x, double y, int w, int h) {
    float ndcX =  2.f * float(x) / float(w) - 1.f;
    float ndcY = -2.f * float(y) / float(h) + 1.f;
    return {ndcX, ndcY};
}

// Inverse View-Projection matrix from camera
inline glm::mat4 invVP(const Camera& c) { return glm::inverse(c.proj()*c.view()); }


// Create a world-space ray from window pixel coords + camera
inline Ray makeRayAtCursor(double x, double y, int w, int h, const Camera& c) {
    glm::vec2 ndc = toNDC(x, y, w, h);
    glm::mat4 invVPMatrix = invVP(c);
    return rayFromNDC(ndc, invVPMatrix);
}
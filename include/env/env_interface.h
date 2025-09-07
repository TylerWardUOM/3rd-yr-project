#pragma once
#include <vector>
#include <cstdint>
#include <cmath>

#include <glm/glm.hpp>

/// Result of an environment query at a point
struct QueryResult {
    double      phi;    // implicit value / signed distance if SDF
    glm::dvec3  grad;   // gradient (not necessarily unit-length)
    glm::dvec3  proj;   // projection of x onto the surface
    bool        inside; // (phi < 0)
};



class EnvInterface {
public:
    virtual ~EnvInterface() = default;

    // Core implicit-surface API (WORLD space)
    virtual double     phi (const glm::dvec3& x) const = 0;
    virtual glm::dvec3 grad(const glm::dvec3& x) const = 0;

    // Default projection using one Newton step: x' = x - f * ∇f / ||∇f||²
    // Override if you have a closed-form projection (e.g., planes).
    virtual glm::dvec3 project(const glm::dvec3& x) const {
        const glm::dvec3 g = grad(x);
        const double g2 = glm::dot(g, g);
        if (g2 <= 1e-18) return x;                 // degenerate gradient; no move
        const double f = phi(x);
        return x - (f / g2) * g;
    }

    // Convenience bundled query
    virtual QueryResult query(const glm::dvec3& x) const {
        QueryResult r;
        r.phi   = phi(x);
        r.grad  = grad(x);
        r.proj  = project(x);
        r.inside = (r.phi < 0.0);
        return r;
    }
};

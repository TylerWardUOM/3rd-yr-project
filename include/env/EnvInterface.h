#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>

/// @defgroup env Environment interface and SDF primitives
/// @brief World-Space implicit surfaces for haptics and collision


/// @ingroup env
/// @brief Result of a full query to an EnvInterface
struct QueryResult {
    double      phi;    // implicit value / signed distance if SDF
    glm::dvec3  grad;   // gradient (not necessarily unit-length)
    glm::dvec3  proj;   // projection of x onto the surface
    bool        inside; // (phi < 0)
};


///@ingroup env
/// @brief Abstract interface for an environment implicit surface
class EnvInterface {
public:
    /// @brief Virtual destructor
    virtual ~EnvInterface() = default;

    /// @brief Signed distance function (SDF) value at WORLD point x.
    /// @details Negative inside, positive outside, zero on surface.
    /// @param x Point in WORLD coordinates 
    /// @return Signed distance in meters.
    virtual double     phi (const glm::dvec3& x) const = 0;

    /// @brief Gradient of phi at WORLD point x (not necessarily unit length).
    /// @param x Point in WORLD coordinates
    /// @return Gradient vector in WORLD coordinates.
    virtual glm::dvec3 grad(const glm::dvec3& x) const = 0;

    /// @brief Project a world-space point toward the surface.
    /// @details Default: single Newton step x' = x - φ ∇φ / ||∇φ||².
    /// Implementations with closed-form projections should override.
    /// @param x World-space point.
    /// @return Approximated closest point on the surface.
    virtual glm::dvec3 project(const glm::dvec3& x) const {
        const glm::dvec3 g = grad(x);
        const double g2 = glm::dot(g, g);
        if (g2 <= 1e-18) return x;                 // degenerate gradient; no move
        const double f = phi(x);
        return x - (f / g2) * g;
    }

    /// @brief  Update world-space parameters using the given LOCAL->WORLD transform.
    /// @param world_from_local 4x4 transform matrix from local to world coordinates.
    virtual void update(const glm::mat4& world_from_local) = 0;

    /// @brief Perform a full query (phi, grad, proj, inside) at WORLD point x.
    /// @param x Point in WORLD coordinates.
    /// @return QueryResult struct with all results.
    virtual QueryResult query(const glm::dvec3& x) const {
        QueryResult r;
        r.phi   = phi(x);
        r.grad  = grad(x);
        r.proj  = project(x);
        r.inside = (r.phi < 0.0);
        return r;
    }
};

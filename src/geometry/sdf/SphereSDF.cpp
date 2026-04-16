#include "geometry/sdf/SphereSDF.h"
#include <glm/glm.hpp>
#include <cmath>

SphereSDF::SphereSDF(const Vec3& center_ls, double radius)
    : c_(center_ls),
      r_(radius > 0.0 ? radius : 0.0)
{}

SDFQuery SphereSDF::queryLocal(const Vec3& p_ls) const {
    SDFQuery q{};

    Vec3 v = {
        p_ls.x - c_.x,
        p_ls.y - c_.y,
        p_ls.z - c_.z
    };

    const double dist = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

    // Signed distance
    q.phi = dist - r_;

    // Gradient (not guaranteed unit length by contract)
    if (dist > 1e-12) {
        q.grad = {
            v.x / dist,
            v.y / dist,
            v.z / dist
        };
    } else {
        // Gradient undefined at center — return stable direction
        q.grad = {1.0, 0.0, 0.0};
    }

    return q;
}

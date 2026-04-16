#pragma once
#include "geometry/sdf/SDF.h"

class SphereSDF : public SDF {
public:
    /// @param center_ls Sphere center in local space
    /// @param radius    Sphere radius (must be > 0)
    SphereSDF(const Vec3& center_ls, double radius);

    SDFQuery queryLocal(const Vec3& p_ls) const override;

private:
    Vec3   c_;   // center (local space)
    double r_;   // radius
};

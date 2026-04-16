// geometry/SDF.h
#pragma once
#include "data/core/Math.h"

struct SDFQuery {
    double phi;    // implicit value / signed distance if SDF
    Vec3   grad;   // gradient (not necessarily unit-length)
    Vec3   proj;   // projection of x onto the surface
    bool   inside; // (phi < 0)
};

class SDF {
public:
    virtual ~SDF() = default;

    virtual SDFQuery queryLocal(const Vec3& p_ls) const = 0;

    // Optional but very useful
    virtual Vec3 projectLocal(const Vec3& p_ls) const {
        SDFQuery q = queryLocal(p_ls);
        return p_ls - q.phi * q.grad;
    }
};

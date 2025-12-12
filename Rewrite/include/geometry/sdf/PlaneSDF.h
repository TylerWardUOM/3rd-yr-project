#pragma once
#include "geometry/sdf/SDF.h"


class PlaneSDF final : public SDF {
public:
    PlaneSDF(const Vec3& n_local, double b_local);

    SDFQuery queryLocal(const Vec3& x_ls) const override;

private:
    Vec3   n_;
    double b_;
};

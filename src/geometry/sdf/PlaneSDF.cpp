#include "geometry/sdf/PlaneSDF.h"


PlaneSDF::PlaneSDF(const Vec3& n_local, double b_local)
    : n_(glm::normalize(n_local)), b_(b_local) {}

SDFQuery PlaneSDF::queryLocal(const Vec3& x_ls) const {
    SDFQuery q;
    q.phi = glm::dot(n_, x_ls) - b_;
    q.grad = n_;
    return q;
}

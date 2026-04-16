// geometry/sdf/UnitSphereSDF.h
#include "geometry/sdf/SDF.h"

class UnitSphereSDF : public SDF {
public:
    SDFQuery queryLocal(const Vec3& p_ls) const override {
        const double r = std::sqrt(p_ls.x*p_ls.x +
                                   p_ls.y*p_ls.y +
                                   p_ls.z*p_ls.z);

        SDFQuery q;
        q.phi = r - 1.0;

        if (r > 1e-12) {
            q.grad = { p_ls.x / r, p_ls.y / r, p_ls.z / r };
        } else {
            q.grad = {1,0,0};
        }
        return q;
    }
};

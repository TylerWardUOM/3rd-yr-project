// geometry/sdf/UnitCubeSDF.h
#include "geometry/sdf/SDF.h"
#include <cmath>
#include <algorithm>

class UnitCubeSDF : public SDF {
public:
    SDFQuery queryLocal(const Vec3& p_ls) const override {

        const double h = 0.5; // unit cube half extent

        const double dx = std::abs(p_ls.x) - h;
        const double dy = std::abs(p_ls.y) - h;
        const double dz = std::abs(p_ls.z) - h;

        const double px = std::max(dx, 0.0);
        const double py = std::max(dy, 0.0);
        const double pz = std::max(dz, 0.0);

        const double outside = std::sqrt(px*px + py*py + pz*pz);
        const double inside  = std::min(std::max({dx, dy, dz}), 0.0);

        SDFQuery q;
        q.phi = outside + inside;

        // -------- gradient --------
        if (outside > 1e-12) {
            // outside cube → gradient toward nearest surface point
            q.grad = {
                (px / outside) * (p_ls.x >= 0 ? 1.0 : -1.0),
                (py / outside) * (p_ls.y >= 0 ? 1.0 : -1.0),
                (pz / outside) * (p_ls.z >= 0 ? 1.0 : -1.0)
            };
        } else {
            // inside cube → normal of nearest face (closest to surface)
            const double m = std::max({dx, dy, dz}); // closest-to-zero (least negative)

            if (m == dx) {
                q.grad = { (p_ls.x >= 0 ? 1.0 : -1.0), 0.0, 0.0 };
            } else if (m == dy) {
                q.grad = { 0.0, (p_ls.y >= 0 ? 1.0 : -1.0), 0.0 };
            } else {
                q.grad = { 0.0, 0.0, (p_ls.z >= 0 ? 1.0 : -1.0) };
            }
        }

        return q;
    }
};

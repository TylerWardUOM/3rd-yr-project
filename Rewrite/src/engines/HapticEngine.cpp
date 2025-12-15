#include "engines/HapticEngine.h"

#include "geometry/GeometryDatabase.h"
#include "geometry/GeometryEntry.h"
#include "geometry/sdf/SDF.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>
// ------------------------------------------------------------
// Small math helpers (local to this TU)
// ------------------------------------------------------------
static inline Vec3 add(const Vec3& a, const Vec3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline Vec3 sub(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline Vec3 mul(const Vec3& a, double s) {
    return {float(a.x * s), float(a.y * s), float(a.z * s)};
}
static inline double dot(const Vec3& a, const Vec3& b) {
    return double(a.x)*b.x + double(a.y)*b.y + double(a.z)*b.z;
}
static inline double norm(const Vec3& v) {
    return std::sqrt(dot(v,v));
}
static inline Vec3 normalize(const Vec3& v) {
    double n = norm(v);
    if (n < 1e-12) return {0,1,0};
    return mul(v, 1.0/n);
}

// ------------------------------------------------------------
// Frame transforms
// ------------------------------------------------------------
static inline Vec3 toLocal(const Pose& T_ws, const Vec3& p_ws) {
    glm::quat q  = glm::quat(T_ws.q);
    glm::quat qi = glm::inverse(q);

    glm::vec3 t = glm::vec3(p_ws.x, p_ws.y, p_ws.z)
                - glm::vec3(T_ws.p.x, T_ws.p.y, T_ws.p.z);

    glm::vec3 pl = qi * t;

    //undo uniform scale
    pl /= float(T_ws.s);

    return {pl.x, pl.y, pl.z};
}


static inline Vec3 toWorld(const Pose& T_ws, const Vec3& p_ls) {
    glm::quat q = glm::quat(T_ws.q);

    glm::vec3 pw = q * (float(T_ws.s) * glm::vec3(p_ls.x, p_ls.y, p_ls.z))
                 + glm::vec3(T_ws.p.x, T_ws.p.y, T_ws.p.z);

    return {pw.x, pw.y, pw.z};
}


static inline Vec3 dirToWorld(const Pose& T_ws, const Vec3& v_ls) {
    glm::quat q = glm::quat(T_ws.q);
    glm::vec3 vw = q * glm::vec3(v_ls.x, v_ls.y, v_ls.z);
    return {vw.x, vw.y, vw.z};
}


// ------------------------------------------------------------
// Constructor / public API
// ------------------------------------------------------------
HapticEngine::HapticEngine(const GeometryDatabase& geomDb,
                           msg::Channel<WorldSnapshot>& worldSnaps,
                           msg::Channel<ToolStateMsg>& toolIn,
                           msg::Channel<HapticSnapshotMsg>& hapticOut,
                           msg::Channel<HapticWrenchCmd>& wrenchOut)
: geometryDb_(geomDb)
, worldSnaps_(worldSnaps)
, toolIn_(toolIn)
, hapticOut_(hapticOut)
, wrenchOut_(wrenchOut)
{
    latestTool_.toolPose_ws = Pose{{0,0,0},{0,0,0,1}};
    proxyPosePrev_          = latestTool_.toolPose_ws;
}


bool HapticEngine::connectDevice(const std::string& port, int baud)
{
    return deviceAdapter_.connect(port, baud);
}

// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------
void HapticEngine::run()
{
    constexpr float dt = 0.001f; // 1 kHz
    while (true) {
        update(dt);
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
}

// ------------------------------------------------------------
// Core haptics update
// ------------------------------------------------------------
void HapticEngine::update(float dt)
{
    // --------------------------------------------------------
    // Drain latest world snapshot
    // --------------------------------------------------------
    WorldSnapshot ws;
    while (worldSnaps_.tryConsume(ws)) {
        latestWorld_ = std::move(ws);
    }

    // --------------------------------------------------------
    // Drain latest tool state
    // --------------------------------------------------------
    ToolStateMsg ti;
    while (toolIn_.tryConsume(ti)) {
        latestTool_ = std::move(ti);
    }

    const Pose& toolPose = latestTool_.toolPose_ws;
    Pose proxyPose = proxyPosePrev_;
    Pose refPose   = toolPose;

    // --------------------------------------------------------
    // Contact search using SDFs
    // --------------------------------------------------------
    double bestPhi = 1e30;
    ObjectID contactId = 0;
    Vec3 contactPoint_ws{0,0,0};
    Vec3 contactNormal_ws{0,1,0};

    for (const ObjectState& obj : latestWorld_.objects) {

        // Skip tool/proxy objects
        if (obj.role == Role::Tool || obj.role == Role::Proxy)
            continue;

        // Geometry lookup (assumes GeometryEntry exposes SDF*)
        const GeometryEntry& geom = geometryDb_.get(obj.geom);
        const SDF* sdf = geom.sdf.get();
        if (!sdf) continue;

        Vec3 p_ls = toLocal(obj.T_ws, toolPose.p);
        SDFQuery q = sdf->queryLocal(p_ls);

        // convert distance to world units
        double phi_ws = q.phi * obj.T_ws.s;
        if (phi_ws < bestPhi) {
            bestPhi = phi_ws;

            if (phi_ws < 0.0) {
                contactId = obj.id;

                Vec3 grad_ls = q.grad;
                double g2 = dot(grad_ls, grad_ls);

                Vec3 proj_ls = p_ls;
                if (g2 > 1e-10 && std::isfinite(q.phi)) {
                    Vec3 n_ls = mul(grad_ls, 1.0 / std::sqrt(g2));
                    proj_ls   = sub(p_ls, mul(n_ls, q.phi)); // local projection
                }

                contactPoint_ws  = toWorld(obj.T_ws, proj_ls);
                contactNormal_ws = normalize(dirToWorld(obj.T_ws, grad_ls));
            }
        }
    }

    // --------------------------------------------------------
    // Proxy projection
    // --------------------------------------------------------
    if (bestPhi < 0.0) {
        // std::cout << "Proxy Position" << proxyPose.p.x << ", " << proxyPose.p.y << ", " << proxyPose.p.z << "\n";
        // std::cout << "Device Position" << toolPose.p.x << ", " << toolPose.p.y << ", " << toolPose.p.z << "\n";
        proxyPose.p = contactPoint_ws;
    } else {
        proxyPose.p = refPose.p;
        contactId = 0;
    }

    // --------------------------------------------------------
    // Virtual coupling (spring–damper)
    // --------------------------------------------------------
    constexpr double K = 2000.0;
    constexpr double M = 0.2;
    const double D = 0.7 * 2.0 * std::sqrt(K * M);

    Vec3 proxyVel = mul(sub(proxyPose.p, proxyPosePrev_.p), 1.0 / dt);
    Vec3 toolVel  = latestTool_.toolVel_ws;

    Vec3 F = add(
        mul(sub(proxyPose.p, toolPose.p), K),
        mul(sub(proxyVel, toolVel), D)
    );

    constexpr double Fmax = 15.0;
    double fn = norm(F);
    if (fn > Fmax) {
        F = mul(F, Fmax / fn);
    }

    // --------------------------------------------------------
    // Publish wrench command (device / physics)
    // --------------------------------------------------------
    if (contactId != 0) {
        HapticWrenchCmd w;
        w.targetId   = contactId;
        w.force_ws   = F;
        w.torque_ws  = {0,0,0};
        w.point_ws   = contactPoint_ws;
        w.duration_s = dt;
        w.t_sec      = latestTool_.t_sec;

        wrenchOut_.publish(w);
        deviceAdapter_.sendWrenchCommand(w);
    }

    // --------------------------------------------------------
    // Publish haptics snapshot
    // --------------------------------------------------------
    HapticSnapshotMsg hs;
    hs.devicePose_ws = toolPose;
    hs.proxyPose_ws  = proxyPose;
    hs.force_ws      = F;
    hs.t_sec         = latestTool_.t_sec;

    hapticOut_.publish(hs);

    proxyPosePrev_ = proxyPose;
}

#include <cmath>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <array>
#include <glm/gtc/epsilon.hpp>

struct basicRobot {
    double link1; // Length of link 1
    double link2; // Length of link 2
};

struct RobotState {
    double jointAngles[3]; // Joint angles for 3 joints
    double endEffectorPos[3]; // End-effector position (x, y, z)
};

RobotState inverseKinematics(basicRobot robot, glm::vec3 target, glm::dvec3& angles) {
    RobotState state;

    double L1 = robot.link1;
    double L2 = robot.link2;

    // Base rotation
    double theta0 = atan2(target.z, target.x);

    // Project into 2D
    double r = sqrt(target.x * target.x + target.z * target.z);
    double y = target.y;

    // Law of cosines for elbow
    double D = (r * r + y * y - L1 * L1 - L2 * L2) / (2 * L1 * L2);
    if (D < -1.0) D = -1.0;
    else if (D > 1.0) D = 1.0;
    double theta2 = -acos(D);

    // Shoulder angle
    double theta1 = atan2(y, r) - atan2(L2 * sin(theta2), L1 + L2 * cos(theta2));

    // Save results
    state.jointAngles[0] = theta0;
    state.jointAngles[1] = theta1;
    state.jointAngles[2] = theta2;

    // End effector pos (optional: forward kinematics with angles)
    //state.endEffectorPos = target;

    angles = glm::dvec3(theta0, theta1, theta2);
    return state;
}


// Rotation matrix (world) from a homogeneous transform:
inline glm::dmat3 rotationOf(const glm::dmat4& T){
    return glm::dmat3(T); // upper-left 3x3 (columns 0..2)
}

// Quaternion (world) from a homogeneous transform:
inline glm::dquat quaternionOf(const glm::dmat4& T){
    return glm::quat_cast(glm::dmat3(T));
}


inline glm::dmat4 Rx(double t){
    double c=std::cos(t), s=std::sin(t);
    glm::dmat4 M(1.0);
    // column-major: M[col][row]
    M[0][0]=1;  M[0][1]=0;  M[0][2]=0;  M[0][3]=0;
    M[1][0]=0;  M[1][1]= c; M[1][2]= s; M[1][3]=0;
    M[2][0]=0;  M[2][1]=-s; M[2][2]= c; M[2][3]=0;
    M[3][0]=0;  M[3][1]=0;  M[3][2]=0;  M[3][3]=1;
    return M;
}

inline glm::dmat4 Ry(double t){
    double c=std::cos(t), s=std::sin(t);
    glm::dmat4 M(1.0);
    M[0][0]= c; M[0][1]=0;  M[0][2]=-s; M[0][3]=0;
    M[1][0]=0;  M[1][1]=1;  M[1][2]=0;  M[1][3]=0;
    M[2][0]= s; M[2][1]=0;  M[2][2]= c; M[2][3]=0;
    M[3][0]=0;  M[3][1]=0;  M[3][2]=0;  M[3][3]=1;
    return M;
}

inline glm::dmat4 Rz(double t){
    double c=std::cos(t), s=std::sin(t);
    glm::dmat4 M(1.0);
    M[0][0]= c; M[0][1]= s; M[0][2]=0; M[0][3]=0;
    M[1][0]=-s; M[1][1]= c; M[1][2]=0; M[1][3]=0;
    M[2][0]=0;  M[2][1]=0;  M[2][2]=1; M[2][3]=0;
    M[3][0]=0;  M[3][1]=0;  M[3][2]=0; M[3][3]=1;
    return M;
}

inline glm::dmat4 Tx(double a){
    glm::dmat4 M(1.0);
    M[3][0]=a; // last column holds translation
    return M;
}

inline std::array<glm::dmat4,4>
forwardExplicitAll(double L1, double L2, const glm::dvec3& th)
{
    // Joint 1 (base) origin: world origin, oriented by Ry(θ0)
    glm::dmat4 T_base = Ry(-th[0]);

    // Joint 2 (shoulder) origin: same position as base in this model (no offset before link1),
    // but oriented by Ry(θ0)*Rz(θ1)
    glm::dmat4 T_shoulder = T_base * Rz(th[1]);

    // Joint 3 (elbow) origin: at end of link1
    glm::dmat4 T_elbow = T_shoulder * Tx(L1);

    // End-effector: rotate at elbow, then translate along link2
    glm::dmat4 T_ee = T_elbow * Rz(th[2]) * Tx(L2);

    return { T_base, T_shoulder, T_elbow, T_ee };
}

inline glm::dquat rotationBetweenVectors_Yup(const glm::dvec3& dir_world){
    // Assumes cylinder local axis is +Y; map +Y to 'dir_world'
    glm::dvec3 v0 = glm::normalize(glm::dvec3(0.0, 1.0, 0.0));
    glm::dvec3 v1 = glm::normalize(dir_world);

    double d = glm::dot(v0, v1);

    // Nearly identical → no rotation
    if (d > 1.0 - 1e-12) return glm::dquat(1.0, 0.0, 0.0, 0.0);

    // Nearly opposite → rotate 180° about any axis ⟂ v0
    if (d < -1.0 + 1e-12) {
        // choose an arbitrary orthogonal axis (prefer x unless collinear)
        glm::dvec3 ortho = glm::abs(v0.x) < 0.9 ? glm::dvec3(1,0,0) : glm::dvec3(0,0,1);
        glm::dvec3 axis = glm::normalize(glm::cross(v0, ortho));
        return glm::angleAxis(glm::pi<double>(), axis);
    }

    // General case
    glm::dvec3 axis = glm::cross(v0, v1);
    double s = std::sqrt((1.0 + d) * 2.0);
    double invs = 1.0 / s;
    return glm::normalize(glm::dquat(
        0.5 * s,                 // w
        axis.x * invs,           // x
        axis.y * invs,           // y
        axis.z * invs            // z
    ));
}


inline Pose linkPoseBetween(const glm::dvec3& A, const glm::dvec3& B){
    glm::dvec3 dir = B - A;
    double len = glm::length(dir);
    if (len < 1e-12) { // degenerate
        return Pose{ A, glm::dquat(1,0,0,0) };
    }
    glm::dvec3 mid = (A + B) ;
    glm::dquat q   = rotationBetweenVectors_Yup(dir);
    return Pose{ A, q };
}

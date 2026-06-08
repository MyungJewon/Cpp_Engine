// 3D 회전 표현과 보간에 사용하는 쿼터니언 연산을 정의합니다.
#pragma once
#include "math/Mat4.h"
#include "math/Vec3.h"
#include <cmath>

struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    static Quat Identity() {
        return {};
    }

    static Quat FromAxisAngle(Vec3 axis, float angle) {
        axis = axis.normalized();
        if (axis.length() <= 0.0f) {
            return Identity();
        }

        const float halfAngle = angle * 0.5f;
        const float s = std::sin(halfAngle);
        return { axis.x * s, axis.y * s, axis.z * s, std::cos(halfAngle) };
    }

    static Quat FromEuler(float pitchDeg, float yawDeg, float rollDeg) {
        constexpr float degToRad = 3.14159265358979323846f / 180.0f;
        const Quat yaw = FromAxisAngle({ 0.0f, 1.0f, 0.0f }, yawDeg * degToRad);
        const Quat pitch = FromAxisAngle({ 1.0f, 0.0f, 0.0f }, pitchDeg * degToRad);
        const Quat roll = FromAxisAngle({ 0.0f, 0.0f, 1.0f }, rollDeg * degToRad);
        return (roll * pitch * yaw).normalized();
    }

    Quat operator*(const Quat& o) const {
        return {
            w * o.x + x * o.w + y * o.z - z * o.y,
            w * o.y - x * o.z + y * o.w + z * o.x,
            w * o.z + x * o.y - y * o.x + z * o.w,
            w * o.w - x * o.x - y * o.y - z * o.z
        };
    }

    Quat conjugate() const {
        return { -x, -y, -z, w };
    }

    Quat normalized() const {
        const float lenSq = x * x + y * y + z * z + w * w;
        if (lenSq <= 0.0f) {
            return Identity();
        }

        const float invLen = 1.0f / std::sqrt(lenSq);
        return { x * invLen, y * invLen, z * invLen, w * invLen };
    }

    Mat4 ToMat4() const {
        const Quat q = normalized();
        const float xx = q.x * q.x;
        const float yy = q.y * q.y;
        const float zz = q.z * q.z;
        const float xy = q.x * q.y;
        const float xz = q.x * q.z;
        const float yz = q.y * q.z;
        const float wx = q.w * q.x;
        const float wy = q.w * q.y;
        const float wz = q.w * q.z;

        Mat4 r = Mat4::Identity();
        r.m[0][0] = 1.0f - 2.0f * (yy + zz);
        r.m[0][1] = 2.0f * (xy - wz);
        r.m[0][2] = 2.0f * (xz + wy);
        r.m[1][0] = 2.0f * (xy + wz);
        r.m[1][1] = 1.0f - 2.0f * (xx + zz);
        r.m[1][2] = 2.0f * (yz - wx);
        r.m[2][0] = 2.0f * (xz - wy);
        r.m[2][1] = 2.0f * (yz + wx);
        r.m[2][2] = 1.0f - 2.0f * (xx + yy);
        return r;
    }

    Vec3 Rotate(const Vec3& v) const {
        const Quat q = normalized();
        const Quat p = { v.x, v.y, v.z, 0.0f };
        const Quat rotated = q * p * q.conjugate();
        return { rotated.x, rotated.y, rotated.z };
    }

    static Quat Slerp(const Quat& a, const Quat& b, float t) {
        Quat from = a.normalized();
        Quat to = b.normalized();
        float dot = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

        if (dot < 0.0f) {
            to = { -to.x, -to.y, -to.z, -to.w };
            dot = -dot;
        }

        if (dot > 0.9995f) {
            return Quat{
                from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t,
                from.z + (to.z - from.z) * t,
                from.w + (to.w - from.w) * t
            }.normalized();
        }

        const float theta0 = std::acos(dot);
        const float theta = theta0 * t;
        const float sinTheta = std::sin(theta);
        const float sinTheta0 = std::sin(theta0);
        const float s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
        const float s1 = sinTheta / sinTheta0;

        return Quat{
            from.x * s0 + to.x * s1,
            from.y * s0 + to.y * s1,
            from.z * s0 + to.z * s1,
            from.w * s0 + to.w * s1
        }.normalized();
    }
};

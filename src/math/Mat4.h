// 3D 변환과 투영에 사용하는 4x4 행렬 연산을 정의합니다.
#pragma once
#include "math/Vec3.h"
#include "math/Vec4.h"
#include <cmath>
#include <cstring>

struct Mat4 {

    float m[4][4];

    Mat4() { memset(m, 0, sizeof(m)); }

    static Mat4 Identity() {
        Mat4 r;
        r.m[0][0] = r.m[1][1] = r.m[2][2] = r.m[3][3] = 1.0f;
        return r;
    }

    static Mat4 Translate(float tx, float ty, float tz) {
        Mat4 r = Identity();
        r.m[0][3] = tx; r.m[1][3] = ty; r.m[2][3] = tz;
        return r;
    }

    static Mat4 Scale(float sx, float sy, float sz) {
        Mat4 r = Identity();
        r.m[0][0] = sx; r.m[1][1] = sy; r.m[2][2] = sz;
        return r;
    }

    static Mat4 Rotate(float angle, Vec3 axis) {
        axis = axis.normalized();
        float c = std::cos(angle), s = std::sin(angle), t = 1.0f - c;
        float x = axis.x, y = axis.y, z = axis.z;
        Mat4 r = Identity();
        r.m[0][0] = t*x*x + c;   r.m[0][1] = t*x*y - s*z; r.m[0][2] = t*x*z + s*y;
        r.m[1][0] = t*x*y + s*z; r.m[1][1] = t*y*y + c;   r.m[1][2] = t*y*z - s*x;
        r.m[2][0] = t*x*z - s*y; r.m[2][1] = t*y*z + s*x; r.m[2][2] = t*z*z + c;
        return r;
    }

    static Mat4 LookAt(Vec3 eye, Vec3 target, Vec3 up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = f.cross(up).normalized();
        Vec3 u = r.cross(f);
        Mat4 res = Identity();
        res.m[0][0] = r.x;  res.m[0][1] = r.y;  res.m[0][2] = r.z;  res.m[0][3] = -r.dot(eye);
        res.m[1][0] = u.x;  res.m[1][1] = u.y;  res.m[1][2] = u.z;  res.m[1][3] = -u.dot(eye);
        res.m[2][0] = -f.x; res.m[2][1] = -f.y; res.m[2][2] = -f.z; res.m[2][3] =  f.dot(eye);
        return res;
    }

    static Mat4 Perspective(float fovY, float aspect, float zNear, float zFar) {
        float tanHalfFov = std::tan(fovY * 0.5f);
        Mat4 r;
        r.m[0][0] = 1.0f / (aspect * tanHalfFov);
        r.m[1][1] = 1.0f / tanHalfFov;
        r.m[2][2] = -(zFar + zNear) / (zFar - zNear);
        r.m[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
        r.m[3][2] = -1.0f;
        return r;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 res;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                for (int k = 0; k < 4; ++k)
                    res.m[i][j] += m[i][k] * o.m[k][j];
        return res;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
            m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w,
        };
    }

    Mat4 NormalMatrix() const {

        Mat4 r = Identity();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = m[i][j];
        return r;
    }
};

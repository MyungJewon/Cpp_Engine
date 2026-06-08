#pragma once
#include "math/Vec3.h"

struct RigidBody {
    Vec3  velocity     = {0, 0, 0};
    Vec3  acceleration = {0, 0, 0};
    Vec3  angularVelocity = {0.0f, 0.0f, 0.0f};  // rad/s
    float mass         = 1.0f;
    float drag         = 0.01f;
    float angularDrag  = 1.5f;                 // 회전 감쇠
    bool  useGravity   = true;
    bool  isKinematic  = false;
};

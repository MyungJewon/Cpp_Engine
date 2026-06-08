// 물리 적분에 필요한 속도와 질량 및 감쇠 값을 정의합니다.
#pragma once
#include "math/Vec3.h"

struct RigidBody {
    Vec3  velocity     = {0, 0, 0};
    Vec3  acceleration = {0, 0, 0};
    Vec3  angularVelocity = {0.0f, 0.0f, 0.0f};
    float mass         = 1.0f;
    float drag         = 0.01f;
    float angularDrag  = 1.5f;
    bool  useGravity   = true;
    bool  isKinematic  = false;
};

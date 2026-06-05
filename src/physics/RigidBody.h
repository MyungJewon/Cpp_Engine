#pragma once
#include "math/Vec3.h"

struct RigidBody {
    Vec3  velocity     = {0, 0, 0};
    Vec3  acceleration = {0, 0, 0};
    float mass         = 1.0f;
    float drag         = 0.01f;
    bool  useGravity   = true;
    bool  isKinematic  = false;
};

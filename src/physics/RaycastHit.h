#pragma once

#include "ecs/Entity.hpp"
#include "math/Vec3.h"

struct RaycastHit {
    Entity entity = NULL_ENTITY;
    float distance = 0.0f;
    Vec3 point;
    Vec3 normal;
    bool hit = false;
};

// Collider와 RigidBody를 처리하는 충돌 시스템을 선언합니다.
#pragma once

#include "ecs/System.hpp"

class CollisionSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

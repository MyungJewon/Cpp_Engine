// RigidBody 기반 물리 적분 시스템을 선언합니다.
#pragma once

#include "ecs/System.hpp"

class PhysicsSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

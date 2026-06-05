#pragma once

#include "ecs/System.hpp"

class PhysicsSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

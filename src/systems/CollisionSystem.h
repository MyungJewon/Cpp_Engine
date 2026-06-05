#pragma once

#include "ecs/System.hpp"

class CollisionSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

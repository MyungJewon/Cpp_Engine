// Registry를 갱신하는 ECS 시스템 인터페이스를 정의합니다.
#pragma once

#include "ecs/Registry.hpp"

class ISystem {
public:

    virtual void update(Registry& reg, float dt) = 0;

    virtual ~ISystem() = default;
};

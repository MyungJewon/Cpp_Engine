#pragma once

#include "ecs/System.hpp"

// ScriptComponent를 가진 Entity의 스크립트를 매 프레임 실행하는 시스템
class ScriptSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

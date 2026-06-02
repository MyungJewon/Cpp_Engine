#pragma once

#include "ecs/System.hpp"

// 매 프레임 입력 상태를 읽어 이벤트를 발행하는 시스템
class InputSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

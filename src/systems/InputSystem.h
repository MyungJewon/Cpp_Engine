// 프레임 입력 이벤트를 발행하는 InputSystem을 선언합니다.
#pragma once

#include "ecs/System.hpp"

class InputSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

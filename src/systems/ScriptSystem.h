// 스크립트 컴포넌트를 매 프레임 실행하는 ScriptSystem을 선언합니다.
#pragma once

#include "ecs/System.hpp"

class ScriptSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

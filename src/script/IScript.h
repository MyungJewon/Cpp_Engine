// Entity에 연결되어 매 프레임 실행되는 스크립트 인터페이스를 정의합니다.
#pragma once

#include "ecs/Entity.hpp"

class Registry;

class IScript {
public:

    virtual void OnInit(Entity self, Registry& reg) {}

    virtual void OnUpdate(Entity self, Registry& reg, float dt) = 0;

    virtual ~IScript() = default;
};

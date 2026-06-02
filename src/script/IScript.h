#pragma once

#include "ecs/Entity.hpp"

class Registry;

// Entity에 붙어서 매 프레임 실행되는 스크립트의 공통 인터페이스
class IScript {
public:
    // 필요하면 스크립트 초기화 시점에 재정의해서 사용한다.
    virtual void OnInit(Entity self, Registry& reg) {}

    // 매 프레임 호출되는 업데이트 함수다.
    virtual void OnUpdate(Entity self, Registry& reg, float dt) = 0;

    virtual ~IScript() = default;
};

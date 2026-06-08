// Transform 계층의 월드 행렬 갱신 시스템을 선언합니다.
#pragma once

#include "ecs/System.hpp"

class TransformSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

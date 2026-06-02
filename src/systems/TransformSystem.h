#pragma once

#include "ecs/System.hpp"

// Transform 계층의 월드 행렬을 프레임마다 최신 상태로 갱신하는 시스템
class TransformSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
};

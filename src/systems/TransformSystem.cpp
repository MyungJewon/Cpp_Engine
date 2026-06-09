// 모든 Transform의 월드 행렬을 갱신하는 시스템 업데이트를 구현합니다.
#include "systems/TransformSystem.h"
#include "ecs/Entity.hpp"
#include "scene/Transform.h"

void TransformSystem::update(Registry& reg, float) {

    for (auto [tf] : reg.view<Transform>()) {
        tf.GetWorldMatrix(reg);
    }
}

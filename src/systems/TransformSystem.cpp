#include "systems/TransformSystem.h"
#include "ecs/Entity.hpp"
#include "scene/Transform.h"

void TransformSystem::update(Registry& reg, float) {
    // 루트 Transform만 순회하면 GetWorldMatrix가 자식 체인을 재귀적으로 갱신한다.
    for (auto [tf] : reg.view<Transform>()) {
        if (tf.parent == NULL_ENTITY) {
            tf.GetWorldMatrix(reg);
        }
    }
}

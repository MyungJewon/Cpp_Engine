// Collider와 RigidBody를 처리하는 충돌 시스템을 선언합니다.
#pragma once

#include "ecs/System.hpp"
#include <set>
#include <utility>

class CollisionSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;

private:
    std::set<std::pair<Entity, Entity>> m_prevContacts;
};

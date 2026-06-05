#include "systems/PhysicsSystem.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"

void PhysicsSystem::update(Registry& reg, float dt) {
    const Vec3 gravity = {0.0f, -9.81f, 0.0f};
    auto& bodies = reg.pool<RigidBody>();

    for (size_t i = 0; i < bodies.size(); ++i) {
        Entity entity = bodies.entity_at(i);
        if (!reg.has<Transform>(entity)) continue;

        RigidBody& rb = bodies.get(entity);
        Transform& transform = reg.get<Transform>(entity);

        if (rb.useGravity) {
            rb.acceleration += gravity;
        }

        if (!rb.isKinematic) {
            rb.velocity += rb.acceleration * dt;
        }

        rb.velocity = rb.velocity * (1.0f - rb.drag * dt);
        transform.SetLocalPos(transform.localPos + rb.velocity * dt, reg);
        rb.acceleration = {0.0f, 0.0f, 0.0f};
    }
}

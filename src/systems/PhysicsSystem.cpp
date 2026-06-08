#include "systems/PhysicsSystem.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
#include <cmath>

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

        float angleSpeed = sqrtf(rb.angularVelocity.x * rb.angularVelocity.x
                               + rb.angularVelocity.y * rb.angularVelocity.y
                               + rb.angularVelocity.z * rb.angularVelocity.z);

        if (angleSpeed > 0.001f) {
            float angle = angleSpeed * dt;
            Vec3 axis = {
                rb.angularVelocity.x / angleSpeed,
                rb.angularVelocity.y / angleSpeed,
                rb.angularVelocity.z / angleSpeed
            };
            Quat deltaRot = Quat::FromAxisAngle(axis, angle);
            transform.SetLocalRot(deltaRot * transform.localRot, reg);
        }

        float angFactor = 1.0f - rb.angularDrag * dt;
        if (angFactor < 0.0f) angFactor = 0.0f;
        rb.angularVelocity.x *= angFactor;
        rb.angularVelocity.y *= angFactor;
        rb.angularVelocity.z *= angFactor;

        rb.acceleration = {0.0f, 0.0f, 0.0f};
    }
}

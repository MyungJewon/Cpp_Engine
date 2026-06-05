#include "systems/CollisionSystem.h"
#include "ecs/Entity.hpp"
#include "event/EventBus.h"
#include "event/Events.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "scene/Transform.h"
#include <algorithm>
#include <cmath>

namespace {
Vec3 WorldPosition(const Transform& transform, const Collider& collider, Registry& reg) {
    Mat4 world = transform.GetWorldMatrix(reg);
    return {world.m[0][3] + collider.center.x,
            world.m[1][3] + collider.center.y,
            world.m[2][3] + collider.center.z};
}

bool AabbVsAabb(const Vec3& aCenter, const Vec3& aHalf,
                const Vec3& bCenter, const Vec3& bHalf,
                Vec3& normal, float& penetration) {
    Vec3 delta = bCenter - aCenter;
    float overlapX = aHalf.x + bHalf.x - std::fabs(delta.x);
    float overlapY = aHalf.y + bHalf.y - std::fabs(delta.y);
    float overlapZ = aHalf.z + bHalf.z - std::fabs(delta.z);

    if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f) {
        return false;
    }

    penetration = overlapX;
    normal = {delta.x < 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f};

    if (overlapY < penetration) {
        penetration = overlapY;
        normal = {0.0f, delta.y < 0.0f ? -1.0f : 1.0f, 0.0f};
    }

    if (overlapZ < penetration) {
        penetration = overlapZ;
        normal = {0.0f, 0.0f, delta.z < 0.0f ? -1.0f : 1.0f};
    }

    return true;
}

bool SphereVsSphere(const Vec3& aCenter, float aRadius,
                    const Vec3& bCenter, float bRadius,
                    Vec3& normal, float& penetration) {
    Vec3 delta = bCenter - aCenter;
    float distance = delta.length();
    float radiusSum = aRadius + bRadius;

    if (distance >= radiusSum) {
        return false;
    }

    normal = distance > 0.0f ? delta / distance : Vec3{1.0f, 0.0f, 0.0f};
    penetration = radiusSum - distance;
    return true;
}

bool SphereVsAabb(const Vec3& sphereCenter, float sphereRadius,
                  const Vec3& boxCenter, const Vec3& boxHalf,
                  Vec3& normal, float& penetration) {
    const Vec3 closest = {
        std::clamp(sphereCenter.x, boxCenter.x - boxHalf.x, boxCenter.x + boxHalf.x),
        std::clamp(sphereCenter.y, boxCenter.y - boxHalf.y, boxCenter.y + boxHalf.y),
        std::clamp(sphereCenter.z, boxCenter.z - boxHalf.z, boxCenter.z + boxHalf.z)
    };

    const Vec3 boxToSphere = sphereCenter - closest;
    const float distance = boxToSphere.length();
    if (distance > sphereRadius) {
        return false;
    }

    if (distance > 0.0f) {
        normal = -boxToSphere / distance;
        penetration = sphereRadius - distance;
        return true;
    }

    const Vec3 delta = sphereCenter - boxCenter;
    const float overlapX = boxHalf.x + sphereRadius - std::fabs(delta.x);
    const float overlapY = boxHalf.y + sphereRadius - std::fabs(delta.y);
    const float overlapZ = boxHalf.z + sphereRadius - std::fabs(delta.z);

    penetration = overlapX;
    normal = {delta.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f};

    if (overlapY < penetration) {
        penetration = overlapY;
        normal = {0.0f, delta.y < 0.0f ? 1.0f : -1.0f, 0.0f};
    }

    if (overlapZ < penetration) {
        penetration = overlapZ;
        normal = {0.0f, 0.0f, delta.z < 0.0f ? 1.0f : -1.0f};
    }

    return true;
}

bool IsMovable(Registry& reg, Entity entity) {
    return reg.has<RigidBody>(entity) && !reg.get<RigidBody>(entity).isKinematic;
}

void MoveEntity(Registry& reg, Entity entity, const Vec3& offset) {
    if (!reg.has<Transform>(entity)) return;

    Transform& transform = reg.get<Transform>(entity);
    transform.SetLocalPos(transform.localPos + offset, reg);
}

void ResolveVelocity(Registry& reg, Entity entity, const Vec3& normal, float restitution, float friction) {
    if (!reg.has<RigidBody>(entity)) return;

    RigidBody& rb = reg.get<RigidBody>(entity);
    const Vec3& vel = rb.velocity;
    const Vec3& n = normal;
    float vDotN = vel.x * n.x + vel.y * n.y + vel.z * n.z;
    float normalImpulse = -(1.0f + restitution) * vDotN;
    if (normalImpulse <= 0.0f) return;

    Vec3 normalDelta = {n.x * normalImpulse, n.y * normalImpulse, n.z * normalImpulse};
    Vec3 tangential = {vel.x - n.x * vDotN, vel.y - n.y * vDotN, vel.z - n.z * vDotN};
    float tangentialSpeed = sqrtf(tangential.x * tangential.x +
                                  tangential.y * tangential.y +
                                  tangential.z * tangential.z);
    friction = std::clamp(friction, 0.0f, 1.0f);
    if (tangentialSpeed > 0.001f) {
        float frictionMag = friction * std::abs(normalImpulse);
        frictionMag = std::min(frictionMag, tangentialSpeed);
        Vec3 tangDir = {tangential.x / tangentialSpeed,
                        tangential.y / tangentialSpeed,
                        tangential.z / tangentialSpeed};
        Vec3 frictionDelta = {tangDir.x * (-frictionMag),
                              tangDir.y * (-frictionMag),
                              tangDir.z * (-frictionMag)};
        rb.velocity = rb.velocity + normalDelta + frictionDelta;
    } else {
        rb.velocity = rb.velocity + normalDelta;
    }
}

void ResolveCollision(Registry& reg, Entity a, Entity b, const Collider& colliderA, const Collider& colliderB,
                      const Vec3& normal, float penetration) {
    bool hasBodyA = reg.has<RigidBody>(a);
    bool hasBodyB = reg.has<RigidBody>(b);
    if (!hasBodyA && !hasBodyB) return;

    bool moveA = hasBodyA && IsMovable(reg, a);
    bool moveB = hasBodyB && IsMovable(reg, b);
    float restitution = std::max(colliderA.restitution, colliderB.restitution);
    float friction = std::max(colliderA.friction, colliderB.friction);

    if (moveA && moveB) {
        MoveEntity(reg, a, normal * (-penetration * 0.5f));
        MoveEntity(reg, b, normal * ( penetration * 0.5f));
        ResolveVelocity(reg, a, -normal, restitution, friction);
        ResolveVelocity(reg, b, normal, restitution, friction);
    } else if (moveA) {
        MoveEntity(reg, a, normal * -penetration);
        ResolveVelocity(reg, a, -normal, restitution, friction);
    } else if (moveB) {
        MoveEntity(reg, b, normal * penetration);
        ResolveVelocity(reg, b, normal, restitution, friction);
    }
}
}

void CollisionSystem::update(Registry& reg, float) {
    auto& colliders = reg.pool<Collider>();

    for (size_t i = 0; i < colliders.size(); ++i) {
        Entity a = colliders.entity_at(i);
        if (!reg.has<Transform>(a)) continue;

        for (size_t j = i + 1; j < colliders.size(); ++j) {
            Entity b = colliders.entity_at(j);
            if (!reg.has<Transform>(b)) continue;

            Collider& colliderA = colliders.get(a);
            Collider& colliderB = colliders.get(b);

            Vec3 normal = {0.0f, 0.0f, 0.0f};
            float penetration = 0.0f;
            bool collided = false;

            Vec3 centerA = WorldPosition(reg.get<Transform>(a), colliderA, reg);
            Vec3 centerB = WorldPosition(reg.get<Transform>(b), colliderB, reg);

            if (colliderA.shape == ColliderShape::AABB && colliderB.shape == ColliderShape::AABB) {
                collided = AabbVsAabb(centerA, colliderA.halfExtents,
                                      centerB, colliderB.halfExtents,
                                      normal, penetration);
            } else if (colliderA.shape == ColliderShape::Sphere && colliderB.shape == ColliderShape::Sphere) {
                collided = SphereVsSphere(centerA, colliderA.radius,
                                          centerB, colliderB.radius,
                                          normal, penetration);
            } else if (colliderA.shape == ColliderShape::Sphere && colliderB.shape == ColliderShape::AABB) {
                collided = SphereVsAabb(centerA, colliderA.radius,
                                        centerB, colliderB.halfExtents,
                                        normal, penetration);
            } else if (colliderA.shape == ColliderShape::AABB && colliderB.shape == ColliderShape::Sphere) {
                collided = SphereVsAabb(centerB, colliderB.radius,
                                        centerA, colliderA.halfExtents,
                                        normal, penetration);
                normal = -normal;
            }

            if (!collided) continue;

            if (colliderA.isTrigger || colliderB.isTrigger) {
                if (colliderA.isTrigger) {
                    EventBus::Emit(TriggerEnterEvent{a, b});
                }
                if (colliderB.isTrigger) {
                    EventBus::Emit(TriggerEnterEvent{b, a});
                }
            } else {
                EventBus::Emit(CollisionEvent{a, b});
                ResolveCollision(reg, a, b, colliderA, colliderB, normal, penetration);
            }
        }
    }
}

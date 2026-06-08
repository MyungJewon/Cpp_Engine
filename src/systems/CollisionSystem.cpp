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

void ApplyAngularFriction(RigidBody& rb, const Collider& collider, const Vec3& normal, const Vec3& frictionVec) {
    Vec3 r = {
        normal.x * (collider.shape == ColliderShape::Sphere ? collider.radius : collider.halfExtents.x),
        normal.y * (collider.shape == ColliderShape::Sphere ? collider.radius : collider.halfExtents.y),
        normal.z * (collider.shape == ColliderShape::Sphere ? collider.radius : collider.halfExtents.z)
    };

    Vec3 torque = {
        r.y * frictionVec.z - r.z * frictionVec.y,
        r.z * frictionVec.x - r.x * frictionVec.z,
        r.x * frictionVec.y - r.y * frictionVec.x
    };

    float r_size = collider.radius > 0.01f ? collider.radius
                 : (collider.halfExtents.x + collider.halfExtents.y + collider.halfExtents.z) / 3.0f;
    float I = (2.0f / 5.0f) * rb.mass * r_size * r_size;
    if (I < 0.001f) I = 0.001f;

    rb.angularVelocity.x += torque.x / I;
    rb.angularVelocity.y += torque.y / I;
    rb.angularVelocity.z += torque.z / I;
}

void ResolveVelocity(Registry& reg, Entity entity, const Collider& collider, const Vec3& normal, float restitution, float friction) {
    if (!reg.has<RigidBody>(entity)) return;

    RigidBody& rb = reg.get<RigidBody>(entity);
    const Vec3& n = normal;

    // 접촉점 반경 벡터 r (표면 방향)
    float r_size = collider.radius > 0.01f ? collider.radius
                 : (collider.halfExtents.x + collider.halfExtents.y + collider.halfExtents.z) / 3.0f;
    Vec3 r = {-n.x * r_size, -n.y * r_size, -n.z * r_size};

    // 접촉점 속도 = 선속도 + cross(ω, r)
    const Vec3& w = rb.angularVelocity;
    Vec3 angularContrib = {
        w.y * r.z - w.z * r.y,
        w.z * r.x - w.x * r.z,
        w.x * r.y - w.y * r.x
    };
    Vec3 contactVel = {
        rb.velocity.x + angularContrib.x,
        rb.velocity.y + angularContrib.y,
        rb.velocity.z + angularContrib.z
    };

    // 법선 방향 충격 (선속도 기준)
    float vDotN = rb.velocity.x * n.x + rb.velocity.y * n.y + rb.velocity.z * n.z;
    float normalImpulse = -(1.0f + restitution) * vDotN;
    if (normalImpulse <= 0.0f) return;

    // 관성 모멘트 (구체: 2/5 m r²)
    float I = (2.0f / 5.0f) * rb.mass * r_size * r_size;
    if (I < 0.001f) I = 0.001f;

    Vec3 normalDelta = {n.x * normalImpulse, n.y * normalImpulse, n.z * normalImpulse};
    rb.velocity = rb.velocity + normalDelta;

    // 접촉점 접선 속도 (ω 기여 포함)
    float cvDotN = contactVel.x * n.x + contactVel.y * n.y + contactVel.z * n.z;
    Vec3 tangential = {
        contactVel.x - n.x * cvDotN,
        contactVel.y - n.y * cvDotN,
        contactVel.z - n.z * cvDotN
    };
    float tangentialSpeed = sqrtf(tangential.x * tangential.x +
                                  tangential.y * tangential.y +
                                  tangential.z * tangential.z);

    friction = std::clamp(friction, 0.0f, 1.0f);
    if (tangentialSpeed > 0.001f) {
        Vec3 tangDir = {tangential.x / tangentialSpeed,
                        tangential.y / tangentialSpeed,
                        tangential.z / tangentialSpeed};

        // 마찰 충격량 크기 (Coulomb 한계)
        float frictionMag = friction * std::abs(normalImpulse);

        // 선속도+각속도 동시 고려한 유효 질량
        // cross(r, tangDir)
        Vec3 rCrossT = {
            r.y * tangDir.z - r.z * tangDir.y,
            r.z * tangDir.x - r.x * tangDir.z,
            r.x * tangDir.y - r.y * tangDir.x
        };
        float angularMassInv = (rCrossT.x * rCrossT.x + rCrossT.y * rCrossT.y + rCrossT.z * rCrossT.z) / I;
        float effectiveMass = 1.0f / (1.0f / rb.mass + angularMassInv);
        float maxFriction = effectiveMass * tangentialSpeed;
        frictionMag = std::min(frictionMag, maxFriction);

        Vec3 frictionImpulse = {tangDir.x * (-frictionMag),
                                tangDir.y * (-frictionMag),
                                tangDir.z * (-frictionMag)};

        // 선속도에 마찰 충격 적용
        rb.velocity.x += frictionImpulse.x / rb.mass;
        rb.velocity.y += frictionImpulse.y / rb.mass;
        rb.velocity.z += frictionImpulse.z / rb.mass;

        // 각속도에 마찰 충격 적용: Δω = cross(r, J) / I
        Vec3 angDelta = {
            (r.y * frictionImpulse.z - r.z * frictionImpulse.y) / I,
            (r.z * frictionImpulse.x - r.x * frictionImpulse.z) / I,
            (r.x * frictionImpulse.y - r.y * frictionImpulse.x) / I
        };
        rb.angularVelocity.x += angDelta.x;
        rb.angularVelocity.y += angDelta.y;
        rb.angularVelocity.z += angDelta.z;
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
        ResolveVelocity(reg, a, colliderA, -normal, restitution, friction);
        ResolveVelocity(reg, b, colliderB, normal, restitution, friction);
    } else if (moveA) {
        MoveEntity(reg, a, normal * -penetration);
        ResolveVelocity(reg, a, colliderA, -normal, restitution, friction);
    } else if (moveB) {
        MoveEntity(reg, b, normal * penetration);
        ResolveVelocity(reg, b, colliderB, normal, restitution, friction);
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

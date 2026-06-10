// 콜라이더 충돌 감지와 위치 및 속도 보정을 구현합니다.
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

void ResolveVelocity(Registry& reg, Entity a, Entity b, const Collider& colliderA, const Collider& colliderB,
                     const Vec3& normal, float restitution, float friction) {
    RigidBody* rbA = reg.try_get<RigidBody>(a);
    RigidBody* rbB = reg.try_get<RigidBody>(b);

    const bool movableA = rbA && !rbA->isKinematic;
    const bool movableB = rbB && !rbB->isKinematic;
    if (!movableA && !movableB) return;

    const float invMassA = movableA && rbA->mass > 0.0f ? 1.0f / rbA->mass : 0.0f;
    const float invMassB = movableB && rbB->mass > 0.0f ? 1.0f / rbB->mass : 0.0f;
    const float invMassSum = invMassA + invMassB;
    if (invMassSum <= 0.0f) return;

    const float rSizeA = colliderA.radius > 0.01f ? colliderA.radius
                       : (colliderA.halfExtents.x + colliderA.halfExtents.y + colliderA.halfExtents.z) / 3.0f;
    const float rSizeB = colliderB.radius > 0.01f ? colliderB.radius
                       : (colliderB.halfExtents.x + colliderB.halfExtents.y + colliderB.halfExtents.z) / 3.0f;
    const Vec3 rA = { normal.x * rSizeA, normal.y * rSizeA, normal.z * rSizeA };
    const Vec3 rB = { -normal.x * rSizeB, -normal.y * rSizeB, -normal.z * rSizeB };

    const Vec3 velocityA = movableA ? rbA->velocity : Vec3{0.0f, 0.0f, 0.0f};
    const Vec3 velocityB = movableB ? rbB->velocity : Vec3{0.0f, 0.0f, 0.0f};
    const float relativeNormalVelocity = (velocityA - velocityB).dot(normal);
    const float normalImpulseMag = -(1.0f + restitution) * relativeNormalVelocity / invMassSum;
    if (normalImpulseMag >= 0.0f) return;

    const Vec3 normalImpulse = normal * normalImpulseMag;
    if (movableA) rbA->velocity = rbA->velocity + normalImpulse * invMassA;
    if (movableB) rbB->velocity = rbB->velocity - normalImpulse * invMassB;

    float invIA = 0.0f;
    float invIB = 0.0f;
    if (movableA) {
        float iA = (2.0f / 5.0f) * rbA->mass * rSizeA * rSizeA;
        if (iA < 0.001f) iA = 0.001f;
        invIA = 1.0f / iA;
    }
    if (movableB) {
        float iB = (2.0f / 5.0f) * rbB->mass * rSizeB * rSizeB;
        if (iB < 0.001f) iB = 0.001f;
        invIB = 1.0f / iB;
    }

    const Vec3 angularA = movableA ? rbA->angularVelocity.cross(rA) : Vec3{0.0f, 0.0f, 0.0f};
    const Vec3 angularB = movableB ? rbB->angularVelocity.cross(rB) : Vec3{0.0f, 0.0f, 0.0f};
    const Vec3 contactVelA = velocityA + angularA;
    const Vec3 contactVelB = velocityB + angularB;
    const Vec3 relativeContactVel = contactVelA - contactVelB;

    const float cvDotN = relativeContactVel.dot(normal);
    const Vec3 tangential = relativeContactVel - normal * cvDotN;
    const float tangentialSpeed = tangential.length();

    friction = std::clamp(friction, 0.0f, 1.0f);
    if (tangentialSpeed > 0.001f) {
        const Vec3 tangDir = tangential / tangentialSpeed;
        float frictionMag = friction * std::abs(normalImpulseMag);

        const Vec3 rACrossT = rA.cross(tangDir);
        const Vec3 rBCrossT = rB.cross(tangDir);
        const float angularMassInvA = rACrossT.dot(rACrossT) * invIA;
        const float angularMassInvB = rBCrossT.dot(rBCrossT) * invIB;
        const float effectiveMass = 1.0f / (invMassSum + angularMassInvA + angularMassInvB);
        const float maxFriction = effectiveMass * tangentialSpeed;
        frictionMag = std::min(frictionMag, maxFriction);

        const Vec3 frictionImpulse = tangDir * -frictionMag;

        if (movableA) {
            rbA->velocity = rbA->velocity + frictionImpulse * invMassA;
            rbA->angularVelocity = rbA->angularVelocity + rA.cross(frictionImpulse) * invIA;
        }
        if (movableB) {
            rbB->velocity = rbB->velocity - frictionImpulse * invMassB;
            rbB->angularVelocity = rbB->angularVelocity - rB.cross(frictionImpulse) * invIB;
        }
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
        ResolveVelocity(reg, a, b, colliderA, colliderB, normal, restitution, friction);
    } else if (moveA) {
        MoveEntity(reg, a, normal * -penetration);
        ResolveVelocity(reg, a, b, colliderA, colliderB, normal, restitution, friction);
    } else if (moveB) {
        MoveEntity(reg, b, normal * penetration);
        ResolveVelocity(reg, a, b, colliderA, colliderB, normal, restitution, friction);
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

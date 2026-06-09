// Entity의 위치와 회전 및 계층 관계를 저장하는 Transform을 정의합니다.
#pragma once
#include "ecs/Entity.hpp"
#include "math/Mat4.h"
#include "math/Quat.h"
#include "math/Vec3.h"
#include <vector>

class Registry;

inline constexpr Entity INVALID_ENTITY = NULL_ENTITY;

struct Transform {
    Vec3 localPos = { 0.0f, 0.0f, 0.0f };
    Quat localRot;
    Vec3 localScale = { 1.0f, 1.0f, 1.0f };
    Entity parent = INVALID_ENTITY;
    std::vector<Entity> children;
    mutable Mat4 worldMatrix = Mat4::Identity();
    mutable bool dirty = true;

    void SetLocalPos(const Vec3& p, Registry& reg);

    void SetLocalRot(const Quat& q, Registry& reg);

    void SetLocalScale(const Vec3& s, Registry& reg);

    void SetParent(Entity self, Entity newParent, Registry& reg);

    void AddChild(Entity child);

    void RemoveChild(Entity child);

    Mat4 GetLocalMatrix() const;

    Mat4 GetWorldMatrix(const Registry& reg) const;

private:

    void MarkDirty(Registry& reg) const;
};

#pragma once
#include "ecs/Entity.hpp"
#include "math/Mat4.h"
#include "math/Quat.h"
#include "math/Vec3.h"
#include <vector>

class Registry;

// Entity.hpp에는 현재 NULL_ENTITY만 있으므로 Transform에서 쓰는 이름을 같은 값으로 맞춘다.
inline constexpr Entity INVALID_ENTITY = NULL_ENTITY;

struct Transform {
    Vec3 localPos = { 0.0f, 0.0f, 0.0f };
    Quat localRot;  // 기본값은 단위 회전이다.
    Vec3 localScale = { 1.0f, 1.0f, 1.0f };
    Entity parent = INVALID_ENTITY;
    std::vector<Entity> children;
    mutable Mat4 worldMatrix;
    mutable bool dirty = true;

    // 로컬 이동 값을 바꾸고 자식까지 월드 행렬 갱신 필요 상태로 표시한다.
    void SetLocalPos(const Vec3& p, Registry& reg);

    // 로컬 회전 값을 바꾸고 자식까지 월드 행렬 갱신 필요 상태로 표시한다.
    void SetLocalRot(const Quat& q, Registry& reg);

    // 로컬 스케일 값을 바꾸고 자식까지 월드 행렬 갱신 필요 상태로 표시한다.
    void SetLocalScale(const Vec3& s, Registry& reg);

    // 부모 Entity를 교체하고 양쪽 children 목록을 동기화한다.
    void SetParent(Entity self, Entity newParent, Registry& reg);

    // 자식 Entity를 목록 뒤에 추가한다.
    void AddChild(Entity child);

    // 자식 Entity를 목록에서 제거한다.
    void RemoveChild(Entity child);

    // 로컬 TRS 행렬을 만든다.
    Mat4 GetLocalMatrix() const;

    // 필요할 때만 부모 체인을 따라 월드 행렬을 다시 계산한다.
    Mat4 GetWorldMatrix(const Registry& reg) const;

private:
    // 자신과 모든 자식 Transform을 dirty 상태로 표시한다.
    void MarkDirty(Registry& reg) const;
};

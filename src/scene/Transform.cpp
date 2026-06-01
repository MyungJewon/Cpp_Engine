#include "scene/Transform.h"
#include "ecs/Registry.hpp"
#include <algorithm>

void Transform::SetLocalPos(const Vec3& p, Registry& reg) {
    localPos = p;
    MarkDirty(reg);
}

void Transform::SetLocalRot(const Quat& q, Registry& reg) {
    localRot = q;
    MarkDirty(reg);
}

void Transform::SetLocalScale(const Vec3& s, Registry& reg) {
    localScale = s;
    MarkDirty(reg);
}

void Transform::SetParent(Entity self, Entity newParent, Registry& reg) {
    // 기존 부모의 자식 목록에서 자신을 먼저 제거한다.
    if (parent != INVALID_ENTITY && reg.has<Transform>(parent)) {
        reg.get<Transform>(parent).RemoveChild(self);
    }

    parent = newParent;

    // 새 부모가 유효하면 새 부모의 자식 목록에 자신을 추가한다.
    if (parent != INVALID_ENTITY && reg.has<Transform>(parent)) {
        reg.get<Transform>(parent).AddChild(self);
    }

    MarkDirty(reg);
}

void Transform::AddChild(Entity child) {
    children.push_back(child);
}

void Transform::RemoveChild(Entity child) {
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

Mat4 Transform::GetLocalMatrix() const {
    // 이 프로젝트의 Mat4 이동/스케일 생성 함수는 Vec3 대신 float 3개를 받는다.
    return Mat4::Translate(localPos.x, localPos.y, localPos.z)
        * localRot.ToMat4()
        * Mat4::Scale(localScale.x, localScale.y, localScale.z);
}

Mat4 Transform::GetWorldMatrix(const Registry& reg) const {
    if (dirty) {
        const Mat4 localMatrix = GetLocalMatrix();
        Registry& mutableReg = const_cast<Registry&>(reg);

        // 부모가 있으면 부모의 월드 행렬 뒤에 로컬 행렬을 붙인다.
        if (parent != INVALID_ENTITY && mutableReg.has<Transform>(parent)) {
            worldMatrix = mutableReg.get<Transform>(parent).GetWorldMatrix(reg) * localMatrix;
        } else {
            worldMatrix = localMatrix;
        }

        dirty = false;
    }

    return worldMatrix;
}

void Transform::MarkDirty(Registry& reg) const {
    dirty = true;

    // 자식 Transform들도 재귀적으로 갱신 필요 상태로 표시한다.
    for (Entity child : children) {
        if (reg.has<Transform>(child)) {
            reg.get<Transform>(child).MarkDirty(reg);
        }
    }
}

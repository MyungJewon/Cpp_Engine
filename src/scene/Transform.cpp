// Transform 계층의 로컬 및 월드 행렬 갱신을 구현합니다.
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

    if (parent != INVALID_ENTITY && reg.has<Transform>(parent)) {
        reg.get<Transform>(parent).RemoveChild(self);
    }

    parent = newParent;

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

    return Mat4::Translate(localPos.x, localPos.y, localPos.z)
        * localRot.ToMat4()
        * Mat4::Scale(localScale.x, localScale.y, localScale.z);
}

Mat4 Transform::GetWorldMatrix(const Registry& reg) const {
    if (dirty) {
        const Mat4 localMatrix = GetLocalMatrix();
        Registry& mutableReg = const_cast<Registry&>(reg);

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

    for (Entity child : children) {
        if (reg.has<Transform>(child)) {
            reg.get<Transform>(child).MarkDirty(reg);
        }
    }
}

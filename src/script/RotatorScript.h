#pragma once

#include "ecs/Registry.hpp"
#include "math/MathUtils.h"
#include "math/Quat.h"
#include "math/Vec3.h"
#include "scene/Transform.h"
#include "script/IScript.h"

// Y축을 기준으로 Entity의 Transform을 일정한 속도로 회전시키는 스크립트
class RotatorScript : public IScript {
public:
    explicit RotatorScript(float degreesPerSecond)
        : m_radiansPerSecond(DegToRad(degreesPerSecond)) {
    }

    void OnUpdate(Entity self, Registry& reg, float dt) override {
        if (!reg.has<Transform>(self)) {
            return;
        }

        m_angle += m_radiansPerSecond * dt;
        Transform& transform = reg.get<Transform>(self);
        transform.SetLocalRot(Quat::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), m_angle), reg);
    }

private:
    float m_radiansPerSecond = 0.0f;
    float m_angle = 0.0f;
};

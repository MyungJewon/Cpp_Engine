// 카메라 컨트롤러를 ECS 시스템으로 실행하는 CameraSystem을 선언합니다.
#pragma once

#include "ecs/Entity.hpp"
#include "ecs/System.hpp"
#include "event/EventBus.h"
#include "event/Events.h"
#include "scene/CameraController.h"

class CameraSystem : public ISystem {
public:
    CameraSystem(CameraController& controller, Entity cameraEntity);

    void update(Registry& reg, float dt) override;

private:
    CameraController& m_controller;
    Entity m_cameraEntity;
};

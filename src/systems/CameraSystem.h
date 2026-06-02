#pragma once

#include "ecs/Entity.hpp"
#include "ecs/System.hpp"
#include "event/EventBus.h"
#include "event/Events.h"
#include "scene/CameraController.h"

// 입력을 읽어 카메라 모드 전환과 카메라 이동을 처리하는 시스템
class CameraSystem : public ISystem {
public:
    CameraSystem(CameraController& controller, Entity cameraEntity);

    void update(Registry& reg, float dt) override;

private:
    CameraController& m_controller;
    Entity m_cameraEntity;
};

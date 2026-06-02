#include "systems/CameraSystem.h"
#include "input/InputManager.h"
#include "scene/Camera.h"

CameraSystem::CameraSystem(CameraController& controller, Entity cameraEntity)
    : m_controller(controller)
    , m_cameraEntity(cameraEntity) {
    EventBus::Subscribe<CameraModeToggleEvent>([this](const CameraModeToggleEvent&) {
        m_controller.mode = (m_controller.mode == CameraMode::Orbit)
            ? CameraMode::FPS
            : CameraMode::Orbit;
    });
}

void CameraSystem::update(Registry& reg, float dt) {
    // 활성 카메라 Entity가 유효할 때만 컨트롤러를 적용한다.
    if (m_cameraEntity != NULL_ENTITY && reg.has<Camera>(m_cameraEntity)) {
        m_controller.Update(reg.get<Camera>(m_cameraEntity), dt);
    }
}

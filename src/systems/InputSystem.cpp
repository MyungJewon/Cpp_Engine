// 입력 상태를 이벤트로 변환하는 입력 시스템 업데이트를 구현합니다.
#include "systems/InputSystem.h"
#include "event/EventBus.h"
#include "event/Events.h"
#include "input/InputCodes.h"
#include "input/InputManager.h"

void InputSystem::update(Registry& reg, float dt) {
    if (InputManager::Get().JustPressed(KeyCode::Tab))
        EventBus::Emit(CameraModeToggleEvent{});
}

#include "systems/InputSystem.h"
#include "event/EventBus.h"
#include "event/Events.h"
#include "input/InputCodes.h"
#include "input/InputManager.h"

// Tab JustPressed -> CameraModeToggleEvent 발행
void InputSystem::update(Registry& reg, float dt) {
    if (InputManager::Get().JustPressed(KeyCode::Tab))
        EventBus::Emit(CameraModeToggleEvent{});
}

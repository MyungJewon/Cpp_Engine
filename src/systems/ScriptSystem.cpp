// ScriptComponent가 가진 스크립트의 프레임 업데이트를 실행합니다.
#include "systems/ScriptSystem.h"
#include "script/ScriptComponent.h"
#include <cstddef>

void ScriptSystem::update(Registry& reg, float dt) {
    auto& scripts = reg.pool<ScriptComponent>();
    size_t index = 0;

    for (auto [scriptComponent] : reg.view<ScriptComponent>()) {
        const Entity entity = scripts.entity_at(index++);
        if (!scriptComponent.script) {
            continue;
        }

        scriptComponent.script->OnUpdate(entity, reg, dt);
    }
}

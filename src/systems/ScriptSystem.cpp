#include "systems/ScriptSystem.h"
#include "script/ScriptComponent.h"
#include <cstddef>

void ScriptSystem::update(Registry& reg, float dt) {
    auto& scripts = reg.pool<ScriptComponent>();
    size_t index = 0;

    // ScriptComponent를 가진 Entity를 순회하면서 연결된 스크립트를 실행한다.
    for (auto [scriptComponent] : reg.view<ScriptComponent>()) {
        const Entity entity = scripts.entity_at(index++);
        if (!scriptComponent.script) {
            continue;
        }

        scriptComponent.script->OnUpdate(entity, reg, dt);
    }
}

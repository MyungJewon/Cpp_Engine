// Registry 컴포넌트를 단일 스레드로 순차 처리하는 헬퍼를 정의합니다.
#pragma once

#include "ecs/Registry.hpp"

template<typename... Ts, typename Func>
void sequential_for(Registry& reg, Func func) {

    auto pools = std::make_tuple(&reg.pool<Ts>()...);
    size_t total = std::get<0>(pools)->size();
    for (size_t i = 0; i < total; ++i) {
        Entity e = std::get<0>(pools)->entity_at(i);

        if ((std::get<ComponentPool<Ts>*>(pools)->has(e) && ...))
            func(e, std::get<ComponentPool<Ts>*>(pools)->get(e)...);
    }
}

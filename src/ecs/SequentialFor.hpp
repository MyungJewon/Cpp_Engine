#pragma once
// =====================================================================
// SequentialFor.hpp — 싱글스레드 순차 처리
//
// [이 파일이 필요한 이유]
// parallel_for와 동일한 API로 싱글스레드 처리를 제공
// main.cpp에서 두 방식의 코드 구조가 동일하게 보이도록 하여
// 순수하게 싱글스레드 vs 멀티스레드 성능만 비교 가능하게 함
// =====================================================================

#include "ecs/Registry.hpp"

// parallel_for와 동일한 인터페이스 — 내부는 단순 순차 루프
// Ts — 처리할 컴포넌트 타입 목록, func — (Entity e, Ts&...) 를 받는 람다
template<typename... Ts, typename Func>
void sequential_for(Registry& reg, Func func) {
    // 풀 포인터를 순회 시작 전에 한 번만 꺼냄
    // reg.pool<T>() 는 unordered_map 탐색을 수반하므로 루프 안에서 반복 호출하면 손해
    auto pools = std::make_tuple(&reg.pool<Ts>()...);
    size_t total = std::get<0>(pools)->size();
    for (size_t i = 0; i < total; ++i) {
        Entity e = std::get<0>(pools)->entity_at(i);
        // Ts... 모두 가진 Entity만 처리
        if ((std::get<ComponentPool<Ts>*>(pools)->has(e) && ...))
            func(e, std::get<ComponentPool<Ts>*>(pools)->get(e)...);
    }
}

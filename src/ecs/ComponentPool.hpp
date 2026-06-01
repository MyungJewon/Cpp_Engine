#pragma once
// =====================================================================
// ComponentPool.hpp — 컴포넌트 데이터를 저장하는 핵심 자료구조
//
// [이 파일이 필요한 이유]
// ECS의 성능 핵심은 같은 종류의 데이터를 메모리에 연속으로 모아두는 것
// C#의 Enemy 클래스처럼 객체가 흩어져 있으면 CPU 캐시 미스가 발생해 느림
// ComponentPool은 Position, Health 등 같은 타입의 데이터를
// 연속 배열(vector)에 모아두어 캐시 효율을 극대화함
//
// [Sparse Set 구조]
// sparse[entity]  → dense 배열에서의 인덱스 (없으면 SIZE_MAX)
// dense[i]        → i번째 슬롯을 소유한 Entity ID
// components[i]   → dense[i] Entity의 실제 컴포넌트 데이터
//
// dense와 components는 항상 같은 크기, 같은 인덱스로 1:1 대응됨
// =====================================================================

#include "ecs/Entity.hpp"
#include <vector>   // std::vector — C#의 List와 동일, 크기가 자동으로 늘어나는 배열
#include <cassert>  // assert() — 조건이 false면 프로그램을 즉시 종료시켜 버그를 잡음

// template<typename T> — T 자리에 Position, Health, Velocity 등 어떤 타입이든 들어올 수 있음
// 덕분에 ComponentPool 하나로 모든 컴포넌트 타입을 처리 가능 (C# generic과 동일)
template<typename T>
class ComponentPool {
public:
    // 특정 Entity에 컴포넌트를 추가
    void add(Entity entity, T component) {
        // 같은 컴포넌트를 두 번 추가하면 버그 → assert로 즉시 감지
        assert(!has(entity) && "Entity already has this component");

        // sparse는 entity ID를 인덱스로 쓰는 배열
        // entity ID가 배열 크기보다 크면 배열을 늘려야 함
        // SIZE_MAX로 채우는 이유: "아직 없음"을 표시하는 sentinel 값
        if (entity >= sparse.size())
            sparse.resize(entity + 1, SIZE_MAX);

        sparse[entity] = dense.size();        // sparse에 "dense의 몇 번째에 들어갈지" 기록
        dense.push_back(entity);              // dense에 entity ID 추가
        components.push_back(std::move(component)); // components에 실제 데이터 추가
        // std::move — 복사 대신 이동. C#에는 없는 개념, 불필요한 복사를 막아 성능 향상
    }

    // 특정 Entity의 컴포넌트를 제거
    void remove(Entity entity) {
        assert(has(entity) && "Entity does not have this component");

        size_t idx  = sparse[entity]; // 제거할 슬롯의 인덱스
        Entity last = dense.back();   // 마지막 슬롯의 Entity ID

        // [O(1) 삭제 트릭 — swap and pop]
        // 중간을 비우면 배열에 구멍이 생김 → 순회할 때 구멍을 건너뛰어야 해서 느려짐
        // 대신 제거할 자리에 마지막 요소를 덮어쓰고 마지막을 pop하면 구멍 없이 삭제 가능
        dense[idx]      = last;                          // 제거할 자리에 마지막 Entity 덮어쓰기
        components[idx] = std::move(components.back());  // 제거할 자리에 마지막 컴포넌트 덮어쓰기
        sparse[last]    = idx;                           // 마지막 Entity의 sparse 인덱스 업데이트

        dense.pop_back();          // 마지막 자리 제거
        components.pop_back();     // 마지막 자리 제거
        sparse[entity] = SIZE_MAX; // 제거한 Entity는 "없음"으로 표시
    }

    // 특정 Entity의 컴포넌트를 가져옴 (수정 가능)
    T& get(Entity entity) {
        assert(has(entity) && "Entity does not have this component");
        return components[sparse[entity]]; // sparse로 인덱스 찾아서 components에서 반환
    }

    // 특정 Entity의 컴포넌트를 가져옴 (읽기 전용)
    // C#의 readonly와 비슷한 역할, const가 붙으면 이 함수 안에서 값을 바꿀 수 없음
    const T& get(Entity entity) const {
        assert(has(entity) && "Entity does not have this component");
        return components[sparse[entity]];
    }

    // 특정 Entity가 이 컴포넌트를 갖고 있는지 확인
    bool has(Entity entity) const {
        // entity ID가 sparse 범위 안에 있고, SIZE_MAX가 아니면 존재하는 것
        return entity < sparse.size() && sparse[entity] != SIZE_MAX;
    }

    // 현재 풀에 들어있는 컴포넌트 개수
    size_t size() const { return components.size(); }

    // range-based for 가능하게 하는 begin/end
    // C#의 IEnumerable과 같은 역할 — "foreach로 순회 가능하게 해줌"
    auto begin()       { return components.begin(); }
    auto end()         { return components.end(); }
    auto begin() const { return components.cbegin(); }
    auto end()   const { return components.cend(); }

    // i번째 슬롯의 Entity ID를 반환 — View에서 "이 컴포넌트의 주인이 누구?"를 알기 위해 사용
    Entity entity_at(size_t idx) const { return dense[idx]; }

private:
    std::vector<size_t> sparse;     // [entity_id] → dense 배열의 인덱스 (없으면 SIZE_MAX)
    std::vector<Entity> dense;      // [인덱스] → entity_id (components와 1:1 대응)
    std::vector<T>      components; // [인덱스] → 실제 컴포넌트 데이터 (연속 배열, 캐시 친화적)
};

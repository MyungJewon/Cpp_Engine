#pragma once
// =====================================================================
// View.hpp — 여러 컴포넌트를 동시에 가진 Entity만 걸러서 순회
//
// [이 파일이 필요한 이유]
// Position과 Velocity를 둘 다 가진 Entity만 처리하려면
// 매번 if (!reg.has<Position>(e)) continue; 같은 코드를 반복해야 함
// View는 이 반복 코드를 내부에서 처리하고
// for (auto [pos, vel] : reg.view<Position, Velocity>()) 처럼
// 깔끔하게 순회할 수 있게 해줌
//
// [C#에서 비슷한 것]
// LINQ의 .Where().Select() 체인과 비슷한 역할
// entities.Where(e => e.Has<Position>() && e.Has<Velocity>())
//         .Select(e => (e.Get<Position>(), e.Get<Velocity>()))
// =====================================================================

#include "ecs/ComponentPool.hpp"
#include <tuple> // std::tuple — C#의 ValueTuple과 동일, 여러 값을 묶어서 반환할 때 사용

// typename... Ts — 타입을 몇 개든 받을 수 있음 (C#의 params와 유사하지만 타입에 적용)
// View<Position>                    — Position 하나
// View<Position, Velocity>          — Position + Velocity
// View<Position, Velocity, Health>  — 세 개 동시에
template<typename... Ts>
class View {
    // 각 컴포넌트 풀의 포인터를 튜플로 보관
    // ex) View<Position, Velocity>라면
    // tuple<ComponentPool<Position>*, ComponentPool<Velocity>*>
    std::tuple<ComponentPool<Ts>*...> pools;

    // entity가 모든 컴포넌트를 갖고 있는지 확인
    // && ... 은 fold expression — Ts 개수만큼 has()를 &&로 자동 연결
    // ex) Ts = Position, Velocity라면:
    // pool<Position>->has(e) && pool<Velocity>->has(e)
    bool qualifies(Entity e) const {
        return (std::get<ComponentPool<Ts>*>(pools)->has(e) && ...);
    }

public:
    // 생성자 — 각 컴포넌트 풀의 참조를 받아서 포인터로 저장
    explicit View(ComponentPool<Ts>&... ps) : pools(&ps...) {}

    // Iterator — range-based for가 동작하려면 반드시 필요
    // C#의 IEnumerator와 동일한 역할
    class Iterator {
        View* view;  // 부모 View에 대한 포인터 (풀들에 접근하기 위해)
        size_t idx;  // 현재 첫 번째 풀에서 보고 있는 인덱스

        // 조건을 만족하지 않는 Entity는 건너뜀
        // 첫 번째 풀을 기준으로 순회하면서 나머지 풀에도 있는지 확인
        void advance() {
            // std::get<0> — 튜플의 첫 번째 요소 (C#의 tuple.Item1에 해당하지만 0부터 시작)
            auto& primary = *std::get<0>(view->pools); // 첫 번째 풀을 기준으로 순회
            // 첫 번째 풀 끝까지 갔거나, 현재 Entity가 모든 컴포넌트를 가질 때까지 이동
            while (idx < primary.size() && !view->qualifies(primary.entity_at(idx)))
                ++idx;
        }

    public:
        Iterator(View* v, size_t i) : view(v), idx(i) {
            // idx가 유효한 범위 안에 있을 때만 advance() 실행
            // end 이터레이터는 advance() 호출 안 함 (범위 초과 방지)
            if (idx < std::get<0>(view->pools)->size())
                advance();
        }

        // * 연산자 — for문에서 현재 값을 꺼낼 때 호출됨
        // 현재 Entity의 컴포넌트들을 tuple of references로 반환
        // auto [pos, vel] = *iterator 처럼 구조분해 가능
        std::tuple<Ts&...> operator*() const {
            Entity e = std::get<0>(view->pools)->entity_at(idx); // 현재 인덱스의 Entity ID
            // std::tie — 여러 참조를 tuple로 묶어서 반환
            // Ts 개수만큼 get()을 자동으로 호출해서 각 컴포넌트 참조를 묶음
            return std::tie(std::get<ComponentPool<Ts>*>(view->pools)->get(e)...);
        }

        // ++ 연산자 — for문에서 다음으로 이동할 때 호출됨
        Iterator& operator++() { ++idx; advance(); return *this; }

        // != 연산자 — for문에서 끝에 도달했는지 확인할 때 호출됨
        bool operator!=(const Iterator& o) const { return idx != o.idx; }
    };

    // range-based for가 동작하려면 begin()과 end()가 반드시 있어야 함
    // C#의 GetEnumerator()와 동일한 역할
    Iterator begin() { return Iterator(this, 0); }                                   // 첫 번째부터 시작
    Iterator end()   { return Iterator(this, std::get<0>(pools)->size()); } // 첫 번째 풀의 끝
};

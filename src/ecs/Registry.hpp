#pragma once
// =====================================================================
// Registry.hpp — ECS의 중앙 관리자
//
// [이 파일이 필요한 이유]
// ComponentPool은 타입마다 따로 존재함 (ComponentPool<Position>, ComponentPool<Health>...)
// 사용자가 이걸 직접 관리하면 풀이 10개, 20개 될 때 매우 복잡해짐
// Registry가 모든 풀을 내부에서 관리하고
// reg.add<Position>(entity, {...}) 처럼 깔끔한 API만 외부에 제공함
//
// [타입 소거 문제]
// ComponentPool<Position>과 ComponentPool<Health>는 서로 다른 타입이라
// 하나의 map에 넣을 수 없음
// 해결책: 공통 부모 IComponentPool을 만들어서 부모 포인터로 저장
// =====================================================================

#include "ecs/Entity.hpp"
#include "ecs/ComponentPool.hpp"
#include "ecs/View.hpp"
#include <unordered_map> // C#의 Dictionary와 동일 — 키-값 쌍으로 빠른 검색
#include <typeindex>     // typeid(Position) 같은 타입 정보를 map의 키로 쓰기 위한 헤더
#include <memory>        // unique_ptr (스마트 포인터) — 자동 메모리 해제를 위해

// [타입 소거를 위한 추상 베이스 클래스]
// C#의 interface와 동일한 역할
// ComponentPool<Position>, ComponentPool<Health> 등 서로 다른 타입을
// IComponentPool* 하나의 타입으로 통일해서 map에 담기 위해 필요
struct IComponentPool {
    virtual ~IComponentPool() = default; // 자식 클래스 소멸자가 제대로 호출되도록 virtual 필수
    virtual void remove(Entity entity) = 0; // = 0 은 "반드시 자식이 구현해야 함" — C#의 abstract
    virtual bool has(Entity entity) const = 0;
};

// IComponentPool을 상속해서 실제 ComponentPool<T>를 감싸는 래퍼
// "Position 풀입니다" 라고 타입 정보를 유지하면서 IComponentPool로도 쓸 수 있게 함
template<typename T>
struct TypedComponentPool : IComponentPool {
    ComponentPool<T> pool; // 실제 데이터가 저장되는 풀

    // IComponentPool의 remove/has를 ComponentPool<T>에 위임
    void remove(Entity entity) override { pool.remove(entity); }
    bool has(Entity entity) const override { return pool.has(entity); }
};

// ECS 중앙 관리자 — Entity 생성/삭제 + 모든 컴포넌트 풀을 한 곳에서 관리
class Registry {
public:
    // 새 Entity ID를 발급해서 반환
    Entity create() {
        return entity_manager.create();
    }

    // Entity와 그에 딸린 모든 컴포넌트를 제거
    void destroy(Entity entity) {
        // map에 있는 모든 풀을 순회하면서 이 Entity의 컴포넌트를 전부 삭제
        // auto& [type, pool] — C#의 foreach (var (key, value) in dict) 와 동일
        for (auto& [type, pool] : pools) {
            if (pool->has(entity))   // 이 풀에 해당 Entity가 있으면
                pool->remove(entity); // 제거
        }
        entity_manager.destroy(entity); // Entity ID를 반납해서 재사용 가능하게
    }

    // Entity에 컴포넌트 추가
    // reg.add<Position>(player, {0.f, 0.f}) 처럼 사용
    template<typename T>
    void add(Entity entity, T component) {
        get_or_create_pool<T>().add(entity, std::move(component));
    }

    // Entity에서 특정 컴포넌트 제거
    template<typename T>
    void remove(Entity entity) {
        get_or_create_pool<T>().remove(entity);
    }

    // Entity의 컴포넌트를 가져옴 (수정 가능한 참조로 반환)
    // C#의 ref 반환과 유사 — 복사가 아닌 원본을 직접 수정 가능
    template<typename T>
    T& get(Entity entity) {
        return get_or_create_pool<T>().get(entity);
    }

    // Entity가 특정 컴포넌트를 갖고 있는지 확인
    template<typename T>
    bool has(Entity entity) {
        auto it = pools.find(typeid(T)); // map에서 이 타입의 풀을 찾음
        if (it == pools.end()) return false; // 풀 자체가 없으면 당연히 없음
        return it->second->has(entity);      // 풀은 있는데 이 Entity가 없을 수도 있음
    }

    // 특정 타입의 ComponentPool을 직접 반환 — View 내부에서 사용
    template<typename T>
    ComponentPool<T>& pool() {
        return get_or_create_pool<T>();
    }

    // 여러 컴포넌트를 동시에 가진 Entity만 순회하는 View 반환
    // reg.view<Position, Velocity>() 처럼 사용
    // ... 은 Ts에 들어온 타입 개수만큼 반복해서 풀을 꺼내는 C++ 문법
    template<typename... Ts>
    View<Ts...> view() {
        return View<Ts...>(get_or_create_pool<Ts>()...);
    }

private:
    // T 타입의 풀이 이미 있으면 반환, 없으면 새로 만들어서 반환
    template<typename T>
    ComponentPool<T>& get_or_create_pool() {
        auto& ptr = pools[typeid(T)]; // map에서 T 타입의 풀을 찾음 (없으면 빈 항목 생성)
        if (!ptr)                      // 빈 항목이면 (아직 만들어진 풀이 없으면)
            ptr = std::make_unique<TypedComponentPool<T>>(); // 새 풀 생성 후 저장
        // unique_ptr 안의 raw 포인터를 꺼내서 TypedComponentPool<T>*로 변환 후 pool 반환
        return static_cast<TypedComponentPool<T>*>(ptr.get())->pool;
    }

    EntityManager entity_manager; // Entity ID 발급/반납 담당

    // 타입 → 풀 매핑 테이블
    // typeid(Position) → TypedComponentPool<Position>
    // typeid(Health)   → TypedComponentPool<Health>
    // unique_ptr 사용 이유: map이 소멸될 때 풀들도 자동으로 메모리 해제되도록
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;
};

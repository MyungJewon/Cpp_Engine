// Entity 생성과 타입별 컴포넌트 풀 관리를 담당하는 Registry를 정의합니다.
#pragma once

#include "ecs/Entity.hpp"
#include "ecs/ComponentPool.hpp"
#include "ecs/View.hpp"
#include <unordered_map>
#include <typeindex>
#include <memory>

struct IComponentPool {
    virtual ~IComponentPool() = default;
    virtual void remove(Entity entity) = 0;
    virtual bool has(Entity entity) const = 0;
};

template<typename T>
struct TypedComponentPool : IComponentPool {
    ComponentPool<T> pool;

    void remove(Entity entity) override { pool.remove(entity); }
    bool has(Entity entity) const override { return pool.has(entity); }
};

class Registry {
public:

    Entity create() {
        return entity_manager.create();
    }

    void destroy(Entity entity) {

        for (auto& [type, pool] : pools) {
            if (pool->has(entity))
                pool->remove(entity);
        }
        entity_manager.destroy(entity);
    }

    template<typename T>
    void add(Entity entity, T component) {
        get_or_create_pool<T>().add(entity, std::move(component));
    }

    template<typename T>
    void remove(Entity entity) {
        get_or_create_pool<T>().remove(entity);
    }

    template<typename T>
    T& get(Entity entity) {
        return get_or_create_pool<T>().get(entity);
    }

    template<typename T>
    T* try_get(Entity entity) {
        auto it = pools.find(std::type_index(typeid(T)));
        if (it == pools.end()) return nullptr;
        auto* pool = static_cast<TypedComponentPool<T>*>(it->second.get());
        if (!pool->has(entity)) return nullptr;
        return &pool->pool.get(entity);
    }

    template<typename T>
    bool has(Entity entity) {
        auto it = pools.find(typeid(T));
        if (it == pools.end()) return false;
        return it->second->has(entity);
    }

    template<typename T>
    ComponentPool<T>& pool() {
        return get_or_create_pool<T>();
    }

    template<typename... Ts>
    View<Ts...> view() {
        return View<Ts...>(get_or_create_pool<Ts>()...);
    }

private:

    template<typename T>
    ComponentPool<T>& get_or_create_pool() {
        auto& ptr = pools[typeid(T)];
        if (!ptr)
            ptr = std::make_unique<TypedComponentPool<T>>();

        return static_cast<TypedComponentPool<T>*>(ptr.get())->pool;
    }

    EntityManager entity_manager;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;
};

// Entity별 컴포넌트를 sparse set 구조로 저장하는 템플릿 풀을 정의합니다.
#pragma once

#include "ecs/Entity.hpp"
#include <vector>
#include <cassert>

template<typename T>
class ComponentPool {
public:

    void add(Entity entity, T component) {

        assert(!has(entity) && "Entity already has this component");

        if (entity >= sparse.size())
            sparse.resize(entity + 1, SIZE_MAX);

        sparse[entity] = dense.size();
        dense.push_back(entity);
        components.push_back(std::move(component));

    }

    void remove(Entity entity) {
        assert(has(entity) && "Entity does not have this component");

        size_t idx  = sparse[entity];
        Entity last = dense.back();

        dense[idx]      = last;
        components[idx] = std::move(components.back());
        sparse[last]    = idx;

        dense.pop_back();
        components.pop_back();
        sparse[entity] = SIZE_MAX;
    }

    T& get(Entity entity) {
        assert(has(entity) && "Entity does not have this component");
        return components[sparse[entity]];
    }

    const T& get(Entity entity) const {
        assert(has(entity) && "Entity does not have this component");
        return components[sparse[entity]];
    }

    bool has(Entity entity) const {

        return entity < sparse.size() && sparse[entity] != SIZE_MAX;
    }

    size_t size() const { return components.size(); }

    auto begin()       { return components.begin(); }
    auto end()         { return components.end(); }
    auto begin() const { return components.cbegin(); }
    auto end()   const { return components.cend(); }

    Entity entity_at(size_t idx) const { return dense[idx]; }

private:
    std::vector<size_t> sparse;
    std::vector<Entity> dense;
    std::vector<T>      components;
};

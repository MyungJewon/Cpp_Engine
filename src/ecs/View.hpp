// 여러 컴포넌트를 동시에 가진 Entity만 순회하는 View를 정의합니다.
#pragma once

#include "ecs/ComponentPool.hpp"
#include <tuple>

template<typename... Ts>
class View {

    std::tuple<ComponentPool<Ts>*...> pools;

    bool qualifies(Entity e) const {
        return (std::get<ComponentPool<Ts>*>(pools)->has(e) && ...);
    }

public:

    explicit View(ComponentPool<Ts>&... ps) : pools(&ps...) {}

    class Iterator {
        View* view;
        size_t idx;

        void advance() {

            auto& primary = *std::get<0>(view->pools);

            while (idx < primary.size() && !view->qualifies(primary.entity_at(idx)))
                ++idx;
        }

    public:
        Iterator(View* v, size_t i) : view(v), idx(i) {

            if (idx < std::get<0>(view->pools)->size())
                advance();
        }

        std::tuple<Ts&...> operator*() const {
            Entity e = std::get<0>(view->pools)->entity_at(idx);

            return std::tie(std::get<ComponentPool<Ts>*>(view->pools)->get(e)...);
        }

        Iterator& operator++() { ++idx; advance(); return *this; }

        bool operator!=(const Iterator& o) const { return idx != o.idx; }
    };

    Iterator begin() { return Iterator(this, 0); }
    Iterator end()   { return Iterator(this, std::get<0>(pools)->size()); }
};

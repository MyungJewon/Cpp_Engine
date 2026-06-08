// ECS Entity ID 타입과 재사용 가능한 Entity 발급기를 정의합니다.
#pragma once

#include <cstdint>
#include <limits>
#include <vector>

using Entity = uint32_t;

inline constexpr Entity NULL_ENTITY = std::numeric_limits<Entity>::max();

class EntityManager {
public:
    Entity create() {

        if (!free_list.empty()) {
            Entity e = free_list.back();
            free_list.pop_back();
            return e;
        }
        return next_id++;
    }

    void destroy(Entity e) {
        free_list.push_back(e);
    }

private:
    Entity next_id = 0;
    std::vector<Entity> free_list;
};

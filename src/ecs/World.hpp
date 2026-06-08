// 여러 ECS 시스템을 등록하고 순서대로 업데이트하는 World를 정의합니다.
#pragma once

#include "ecs/System.hpp"
#include <vector>
#include <memory>
#include <utility>

class World {
public:

    World(Registry& reg) : reg(reg) {}

    template<typename T, typename... Args>
    void add_system(Args&&... args) {

        systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
    void add_fixed_system(Args&&... args) {
        fixedSystems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void update(float dt) {

        for (auto& system : systems)
            system->update(reg, dt);
    }

    void fixed_update(float dt) {
        for (auto& system : fixedSystems)
            system->update(reg, dt);
    }

private:
    Registry& reg;

    std::vector<std::unique_ptr<ISystem>> systems;
    std::vector<std::unique_ptr<ISystem>> fixedSystems;
};

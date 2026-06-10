#pragma once
#include "ecs/System.hpp"

class AudioSystem : public ISystem {
public:
    void update(Registry& reg, float dt) override;
private:
    bool m_initialized = false;
};

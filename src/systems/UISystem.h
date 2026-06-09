// ECS system that draws UIComponent data through UIRenderer.
#pragma once

#include "ecs/System.hpp"
#include "platform/IWindow.h"
#include "ui/UIRenderer.h"

class UISystem : public ISystem {
public:
    UISystem(UIRenderer& renderer, IWindow& window);

    void update(Registry& reg, float dt) override;

private:
    UIRenderer& m_renderer;
    IWindow& m_window;
};

#pragma once

#include "ecs/System.hpp"
#include "platform/IWindow.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"

// 렌더러 호출을 ECS 시스템 실행 순서 안으로 옮기는 시스템
class RenderSystem : public ISystem {
public:
    RenderSystem(Renderer& renderer, Scene& scene, IWindow& window);

    void update(Registry& reg, float dt) override;

private:
    Renderer& m_renderer;
    Scene& m_scene;
    IWindow& m_window;
};

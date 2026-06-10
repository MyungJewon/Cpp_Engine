// Renderer를 ECS 시스템 흐름에서 호출하는 RenderSystem을 선언합니다.
#pragma once

#include "ecs/System.hpp"
#include "platform/IWindow.h"
#include "renderer/Renderer.h"
#include "renderer/Skybox.h"
#include "scene/Scene.h"

class RenderSystem : public ISystem {
public:
    RenderSystem(Renderer& renderer, Scene& scene, IWindow& window, Skybox* skybox = nullptr);

    void SetSkybox(Skybox* skybox) { m_skybox = skybox; }
    void update(Registry& reg, float dt) override;

private:
    Renderer& m_renderer;
    Scene& m_scene;
    IWindow& m_window;
    Skybox* m_skybox = nullptr;
};

// Scene 렌더링 호출을 ECS 시스템 업데이트로 연결합니다.
#include "systems/RenderSystem.h"

RenderSystem::RenderSystem(Renderer& renderer, Scene& scene, IWindow& window)
    : m_renderer(renderer)
    , m_scene(scene)
    , m_window(window) {
}

void RenderSystem::update(Registry&, float) {

    m_renderer.Render(m_scene, m_window);
}

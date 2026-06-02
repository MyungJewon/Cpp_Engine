#include "systems/RenderSystem.h"

RenderSystem::RenderSystem(Renderer& renderer, Scene& scene, IWindow& window)
    : m_renderer(renderer)
    , m_scene(scene)
    , m_window(window) {
}

void RenderSystem::update(Registry&, float) {
    // Scene 기반 렌더 경로를 그대로 사용한다.
    m_renderer.Render(m_scene, m_window);
}

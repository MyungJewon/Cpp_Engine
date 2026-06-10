// Scene 렌더링 호출을 ECS 시스템 업데이트로 연결합니다.
#include "systems/RenderSystem.h"

RenderSystem::RenderSystem(Renderer& renderer, Scene& scene, IWindow& window, Skybox* skybox)
    : m_renderer(renderer)
    , m_scene(scene)
    , m_window(window)
    , m_skybox(skybox) {
}

void RenderSystem::update(Registry&, float) {

    m_renderer.Render(m_scene, m_window);
    if (m_skybox && m_skybox->IsLoaded()) {
        m_skybox->Render(m_scene.GetActiveCamera());
    }
}

// Renders all visible UI components from the registry component pool.
#include "systems/UISystem.h"
#include "ui/UIComponent.h"

UISystem::UISystem(UIRenderer& renderer, IWindow& window)
    : m_renderer(renderer)
    , m_window(window) {
}

void UISystem::update(Registry& reg, float) {
    m_renderer.BeginFrame(m_window.PixelWidth(), m_window.PixelHeight());

    auto& pool = reg.pool<UIComponent>();
    for (auto& ui : pool) {
        if (!ui.visible) continue;

        if (ui.type == UIType::Rect) {
            m_renderer.DrawRect(ui.x, ui.y, ui.width, ui.height, ui.color, ui.alpha);
        } else if (ui.type == UIType::Text) {
            m_renderer.DrawText(ui.x, ui.y, ui.text, ui.fontSize, ui.color, ui.alpha);
        }
    }

    m_renderer.EndFrame();
}

// Renders all visible UI components from the registry component pool.
#include "systems/UISystem.h"
#include "input/InputCodes.h"
#include "input/InputManager.h"
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

    if (InputManager::Get().JustMousePressed(MouseButton::Left)) {
        const float mx = static_cast<float>(InputManager::Get().MouseX());
        const float my = static_cast<float>(InputManager::Get().MouseY());
        for (auto& ui : pool) {
            if (!ui.visible || !ui.onClick) continue;
            if (mx >= ui.x && mx <= ui.x + ui.width &&
                my >= ui.y && my <= ui.y + ui.height) {
                ui.onClick();
            }
        }
    }
}

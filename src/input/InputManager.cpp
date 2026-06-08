// 키와 마우스의 현재 프레임 입력 상태 갱신을 구현합니다.
#include "input/InputManager.h"
#include <algorithm>

InputManager& InputManager::Get() {
    static InputManager instance;
    return instance;
}

bool InputManager::IsValid(KeyCode key) {
    const auto index = static_cast<std::size_t>(key);
    return index < KeyCount;
}

bool InputManager::IsValid(MouseButton button) {
    const auto index = static_cast<std::size_t>(button);
    return index < MouseButtonCount;
}

void InputManager::OnKeyDown(KeyCode key) {
    if (!IsValid(key)) return;
    const auto index = static_cast<std::size_t>(key);

    if (!m_keyDown[index]) m_keyPressed[index] = true;
    m_keyDown[index] = true;
}

void InputManager::OnKeyUp(KeyCode key) {
    if (!IsValid(key)) return;
    const auto index = static_cast<std::size_t>(key);

    if (m_keyDown[index]) m_keyReleased[index] = true;
    m_keyDown[index] = false;
}

void InputManager::OnMouseDown(MouseButton button) {
    if (!IsValid(button)) return;
    m_mouseDown[static_cast<std::size_t>(button)] = true;
}

void InputManager::OnMouseUp(MouseButton button) {
    if (!IsValid(button)) return;
    m_mouseDown[static_cast<std::size_t>(button)] = false;
}

void InputManager::OnMouseMove(int x, int y) {
    if (m_hasMousePosition) {
        m_mouseDX += x - m_mouseX;
        m_mouseDY += y - m_mouseY;
    } else {
        m_hasMousePosition = true;
    }

    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseScroll(float delta) {
    m_scrollDelta += delta;
}

void InputManager::EndFrame() {

    std::fill(m_keyPressed.begin(), m_keyPressed.end(), false);
    std::fill(m_keyReleased.begin(), m_keyReleased.end(), false);
    m_mouseDX = 0;
    m_mouseDY = 0;
    m_scrollDelta = 0.0f;
}

bool InputManager::IsKeyDown(KeyCode key) const {
    return IsValid(key) && m_keyDown[static_cast<std::size_t>(key)];
}

bool InputManager::JustPressed(KeyCode key) const {
    return IsValid(key) && m_keyPressed[static_cast<std::size_t>(key)];
}

bool InputManager::JustReleased(KeyCode key) const {
    return IsValid(key) && m_keyReleased[static_cast<std::size_t>(key)];
}

bool InputManager::IsMouseDown(MouseButton button) const {
    return IsValid(button) && m_mouseDown[static_cast<std::size_t>(button)];
}

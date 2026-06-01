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

    // 처음 눌린 프레임만 JustPressed가 true가 되도록 이전 상태를 확인한다.
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

void InputManager::EndFrame() {
    // 프레임 단위 상태는 루프 끝에서 초기화하고, 누르고 있는 상태는 유지한다.
    std::fill(m_keyPressed.begin(), m_keyPressed.end(), false);
    std::fill(m_keyReleased.begin(), m_keyReleased.end(), false);
    m_mouseDX = 0;
    m_mouseDY = 0;
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

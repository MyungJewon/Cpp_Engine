// 키보드와 마우스 입력 상태를 조회하는 싱글톤 입력 관리자를 선언합니다.
#pragma once
#include "input/InputCodes.h"
#include <array>
#include <cstddef>

class InputManager {
public:
    static InputManager& Get();

    void OnKeyDown(KeyCode key);
    void OnKeyUp(KeyCode key);
    void OnMouseDown(MouseButton button);
    void OnMouseUp(MouseButton button);
    void OnMouseMove(int x, int y);
    void OnMouseScroll(float delta);
    void EndFrame();

    bool IsKeyDown(KeyCode key) const;
    bool JustPressed(KeyCode key) const;
    bool JustReleased(KeyCode key) const;
    bool IsMouseDown(MouseButton button) const;
    bool JustMousePressed(MouseButton button) const;

    int MouseX() const { return m_mouseX; }
    int MouseY() const { return m_mouseY; }
    int MouseDX() const { return m_mouseDX; }
    int MouseDY() const { return m_mouseDY; }
    float ScrollDelta() const { return m_scrollDelta; }

private:
    static constexpr std::size_t KeyCount = static_cast<std::size_t>(KeyCode::Count);
    static constexpr std::size_t MouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

    static bool IsValid(KeyCode key);
    static bool IsValid(MouseButton button);

    std::array<bool, KeyCount> m_keyDown{};
    std::array<bool, KeyCount> m_keyPressed{};
    std::array<bool, KeyCount> m_keyReleased{};
    std::array<bool, MouseButtonCount> m_mouseDown{};
    std::array<bool, MouseButtonCount> m_mousePressed{};

    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_mouseDX = 0;
    int m_mouseDY = 0;
    float m_scrollDelta = 0.0f;
    bool m_hasMousePosition = false;
};

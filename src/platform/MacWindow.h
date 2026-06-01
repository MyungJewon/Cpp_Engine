#pragma once
#include "platform/IWindow.h"
#include <chrono>
#include <memory>

class MacWindow : public IWindow {
public:
    MacWindow(int width, int height, const char* title);
    ~MacWindow() override;

    bool  IsOpen()     const override { return m_open; }
    void  PollEvents() override;                 // 이벤트 폴링 + 델타타임 갱신
    void  Present(const Framebuffer& fb) override; // 프레임버퍼 → CGImage → NSView 출력
    float DeltaTime()  const override { return m_deltaTime; }
    int   Width()      const override { return m_width; }
    int   Height()     const override { return m_height; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    int   m_width     = 0;
    int   m_height    = 0;
    bool  m_open      = true;
    float m_deltaTime = 0.016f;
    std::chrono::steady_clock::time_point m_lastTime;
};

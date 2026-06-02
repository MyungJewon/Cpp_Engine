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
    void  SwapBuffers() override;                 // OpenGL 백버퍼를 화면에 표시
    float DeltaTime()  const override { return m_deltaTime; }
    int   Width()      const override { return m_width; }
    int   Height()     const override { return m_height; }
    int   PixelWidth() const override { return m_pixelWidth; }
    int   PixelHeight()const override { return m_pixelHeight; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    int   m_width      = 0;
    int   m_height     = 0;
    int   m_pixelWidth = 0;   // Retina 실제 픽셀 너비
    int   m_pixelHeight= 0;   // Retina 실제 픽셀 높이
    bool  m_open      = true;
    float m_deltaTime = 0.016f;
    std::chrono::steady_clock::time_point m_lastTime;
};

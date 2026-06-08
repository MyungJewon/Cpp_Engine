// macOS Cocoa 기반 창과 OpenGL 컨텍스트 래퍼를 선언합니다.
#pragma once
#include "platform/IWindow.h"
#include <chrono>
#include <memory>

class MacWindow : public IWindow {
public:
    MacWindow(int width, int height, const char* title);
    ~MacWindow() override;

    bool  IsOpen()     const override { return m_open; }
    void  PollEvents() override;
    void  SwapBuffers() override;
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
    int   m_pixelWidth = 0;
    int   m_pixelHeight= 0;
    bool  m_open      = true;
    float m_deltaTime = 0.016f;
    std::chrono::steady_clock::time_point m_lastTime;
};

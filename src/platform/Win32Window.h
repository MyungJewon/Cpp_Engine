#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "platform/IWindow.h"

class Win32Window : public IWindow {
public:
    Win32Window(int width, int height, const char* title);
    ~Win32Window() override;

    bool  IsOpen()       const override { return m_open; }
    void  PollEvents()         override;
    void  SwapBuffers()        override;
    float DeltaTime()    const override { return m_deltaTime; }
    int   Width()        const override { return m_width; }
    int   Height()       const override { return m_height; }
    int   PixelWidth()   const override { return m_width; }
    int   PixelHeight()  const override { return m_height; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND  m_hwnd      = nullptr;
    HDC   m_hdc       = nullptr;
    HGLRC m_hglrc     = nullptr;
    int   m_width, m_height;
    bool  m_open      = true;

    LARGE_INTEGER m_freq{}, m_lastTime{};
    float m_deltaTime = 0.016f;
};
#endif

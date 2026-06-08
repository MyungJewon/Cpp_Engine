// Win32 창 생성과 메시지 처리 및 입력 전달을 구현합니다.
#include "platform/Win32Window.h"
#include <cstring>
#include <stdexcept>

static Win32Window* g_instance = nullptr;

Win32Window::Win32Window(int width, int height, const char* title)
    : m_width(width), m_height(height) {
    g_instance = this;

    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "SoftRendererWindow";
    RegisterClassExA(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExA(
        0, "SoftRendererWindow", title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, wc.hInstance, nullptr
    );
    if (!m_hwnd) throw std::runtime_error("Failed to create window");

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = width;
    bmi.bmiHeader.biHeight      = -height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC screenDC = GetDC(m_hwnd);
    m_bitmap  = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &m_dibBits, nullptr, 0);
    m_hdc     = CreateCompatibleDC(screenDC);
    SelectObject(m_hdc, m_bitmap);
    ReleaseDC(m_hwnd, screenDC);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_lastTime);
}

Win32Window::~Win32Window() {
    if (m_hdc)    DeleteDC(m_hdc);
    if (m_bitmap) DeleteObject(m_bitmap);
    if (m_hwnd)   DestroyWindow(m_hwnd);
    g_instance = nullptr;
}

void Win32Window::PollEvents() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_deltaTime = (float)(now.QuadPart - m_lastTime.QuadPart) / (float)m_freq.QuadPart;
    m_lastTime  = now;
}

void Win32Window::SwapBuffers() {

}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY || msg == WM_CLOSE) {
        if (g_instance) g_instance->m_open = false;
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_ESCAPE) {
        if (g_instance) g_instance->m_open = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

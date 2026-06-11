#ifdef _WIN32
#include "platform/Win32Window.h"
#include "input/InputCodes.h"
#include "input/InputManager.h"
#include <glad/glad.h>
#include <stdexcept>

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef BOOL (WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);

#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

static Win32Window* g_instance = nullptr;

static KeyCode MapVKey(WPARAM wp) {
    switch (wp) {
        case 'W': return KeyCode::W;
        case 'A': return KeyCode::A;
        case 'S': return KeyCode::S;
        case 'D': return KeyCode::D;
        case 'Q': return KeyCode::Q;
        case 'E': return KeyCode::E;
        case 'R': return KeyCode::R;
        case VK_SPACE:  return KeyCode::Space;
        case VK_SHIFT:  return KeyCode::LeftShift;
        case VK_ESCAPE: return KeyCode::Escape;
        case VK_TAB:    return KeyCode::Tab;
        case VK_UP:     return KeyCode::Up;
        case VK_DOWN:   return KeyCode::Down;
        case VK_LEFT:   return KeyCode::Left;
        case VK_RIGHT:  return KeyCode::Right;
        default:        return KeyCode::Count;
    }
}

Win32Window::Win32Window(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    g_instance = this;
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "CppEngineWindow";
    RegisterClassExA(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    m_hwnd = CreateWindowExA(0, "CppEngineWindow", title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!m_hwnd) throw std::runtime_error("CreateWindowExA failed");
    m_hdc = GetDC(m_hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd); pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 32; pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int fmt = ChoosePixelFormat(m_hdc, &pfd);
    SetPixelFormat(m_hdc, fmt, &pfd);

    HGLRC tempCtx = wglCreateContext(m_hdc);
    wglMakeCurrent(m_hdc, tempCtx);

    auto wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
        throw std::runtime_error("wglCreateContextAttribsARB not available");

    const int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4, WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0
    };
    m_hglrc = wglCreateContextAttribsARB(m_hdc, nullptr, attribs);
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempCtx);
    if (!m_hglrc) throw std::runtime_error("OpenGL 4.1 Core Profile context creation failed");
    wglMakeCurrent(m_hdc, m_hglrc);
    if (!gladLoadGL()) throw std::runtime_error("gladLoadGL failed");
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    QueryPerformanceFrequency(&m_freq);
    QueryPerformanceCounter(&m_lastTime);
}

Win32Window::~Win32Window() {
    if (m_hglrc) { wglMakeCurrent(nullptr, nullptr); wglDeleteContext(m_hglrc); }
    if (m_hdc && m_hwnd) ReleaseDC(m_hwnd, m_hdc);
    if (m_hwnd) DestroyWindow(m_hwnd);
    g_instance = nullptr;
}

void Win32Window::PollEvents() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg); DispatchMessageA(&msg);
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    m_deltaTime = static_cast<float>(now.QuadPart - m_lastTime.QuadPart)
                / static_cast<float>(m_freq.QuadPart);
    m_lastTime = now;
    InputManager::Get().EndFrame();
}

void Win32Window::SwapBuffers() { ::SwapBuffers(m_hdc); }

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_DESTROY: case WM_CLOSE:
            if (g_instance) g_instance->m_open = false;
            PostQuitMessage(0); return 0;
        case WM_KEYDOWN: {
            KeyCode kc = MapVKey(wp);
            if (kc == KeyCode::Escape) { if (g_instance) g_instance->m_open = false; PostQuitMessage(0); }
            else if (kc != KeyCode::Count) InputManager::Get().OnKeyDown(kc);
            return 0;
        }
        case WM_KEYUP: { KeyCode kc = MapVKey(wp); if (kc != KeyCode::Count) InputManager::Get().OnKeyUp(kc); return 0; }
        case WM_LBUTTONDOWN: InputManager::Get().OnMouseDown(MouseButton::Left); return 0;
        case WM_LBUTTONUP:   InputManager::Get().OnMouseUp(MouseButton::Left); return 0;
        case WM_RBUTTONDOWN: InputManager::Get().OnMouseDown(MouseButton::Right); return 0;
        case WM_RBUTTONUP:   InputManager::Get().OnMouseUp(MouseButton::Right); return 0;
        case WM_MOUSEMOVE:   InputManager::Get().OnMouseMove(LOWORD(lp), HIWORD(lp)); return 0;
        case WM_MOUSEWHEEL: {
            float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wp)) / WHEEL_DELTA;
            InputManager::Get().OnMouseScroll(delta); return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
#endif

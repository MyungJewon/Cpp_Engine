#include "app/Application.h"
#include "app/GameLoop.h"

#ifdef _WIN32
#include "platform/Win32Window.h"
#else
#include "platform/MacWindow.h"
#endif

Application::Application(int width, int height, const char* title)
{
#ifdef _WIN32
    // Windows에서는 Win32 메시지 펌프 기반 창을 생성한다.
    m_window = std::make_unique<Win32Window>(width, height, title);
#else
    // macOS에서는 Cocoa 기반 픽셀 출력 창을 생성한다.
    m_window = std::make_unique<MacWindow>(width, height, title);
#endif
}

Application::~Application() = default;

void Application::Run() {
    GameLoop::Run(*this);
}

void Application::OnRender() {
}

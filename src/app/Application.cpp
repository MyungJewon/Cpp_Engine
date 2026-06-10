// 애플리케이션 창 생성과 실행 루프 진입을 구현합니다.
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

    m_window = std::make_unique<Win32Window>(width, height, title);
#else

    m_window = std::make_unique<MacWindow>(width, height, title);
#endif
}

Application::~Application() = default;

void Application::Run() {
    GameLoop::Run(*this);
}

void Application::LoadScene(std::unique_ptr<IScene> scene) {
    m_pendingScene = std::move(scene);
}

void Application::OnUpdate(float dt) {
    if (m_pendingScene) {
        if (m_currentScene) m_currentScene->OnExit();
        m_currentScene = std::move(m_pendingScene);
        m_currentScene->SetApplication(this);
        m_currentScene->OnEnter();
    }
    if (m_currentScene) m_currentScene->OnUpdate(dt);
}

void Application::OnFixedUpdate() {
    if (m_currentScene) m_currentScene->OnFixedUpdate();
}

void Application::OnRender() {
    if (m_currentScene) m_currentScene->OnRender();
}

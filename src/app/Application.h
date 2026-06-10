// 플랫폼 창을 소유하고 게임 루프 콜백을 제공하는 기본 애플리케이션을 선언합니다.
#pragma once
#include "app/IScene.h"
#include "platform/IWindow.h"
#include <memory>

class GameLoop;

class Application {
public:
    Application(int width, int height, const char* title);
    virtual ~Application();

    void Run();
    void LoadScene(std::unique_ptr<IScene> scene);

    IWindow& GetWindow() { return *m_window; }
    const IWindow& GetWindow() const { return *m_window; }

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float);
    virtual void OnFixedUpdate();
    virtual void OnRender();

private:
    friend class GameLoop;

    std::unique_ptr<IWindow> m_window;
    std::unique_ptr<IScene> m_currentScene;
    std::unique_ptr<IScene> m_pendingScene;
};

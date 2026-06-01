#pragma once
#include "platform/IWindow.h"
#include "renderer/Framebuffer.h"
#include <memory>

class GameLoop;

class Application {
public:
    Application(int width, int height, const char* title);
    virtual ~Application();

    void Run();

    IWindow& GetWindow() { return *m_window; }
    const IWindow& GetWindow() const { return *m_window; }
    Framebuffer& GetFramebuffer() { return m_framebuffer; }
    const Framebuffer& GetFramebuffer() const { return m_framebuffer; }

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float) {}
    virtual void OnFixedUpdate() {}
    virtual void OnRender();

private:
    friend class GameLoop;

    std::unique_ptr<IWindow> m_window;
    Framebuffer m_framebuffer;
};

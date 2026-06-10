// 씬 전환을 위한 씬 인터페이스를 정의합니다.
#pragma once

class Application;

class IScene {
public:
    virtual ~IScene() = default;

    virtual void OnEnter() {}
    virtual void OnExit()  {}
    virtual void OnUpdate(float dt) = 0;
    virtual void OnFixedUpdate() {}
    virtual void OnRender() {}

    void SetApplication(Application* app) { m_app = app; }

protected:
    Application* m_app = nullptr;
};

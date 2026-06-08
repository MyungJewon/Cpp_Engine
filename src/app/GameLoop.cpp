// 고정 업데이트와 프레임 업데이트를 구동하는 메인 게임 루프를 구현합니다.
#include "app/GameLoop.h"
#include "app/Application.h"
#include "core/Time.h"
#include "input/InputManager.h"

void GameLoop::Run(Application& app) {
    Time::Init();
    app.OnInit();

    auto& window = app.GetWindow();
    float fixedAcc = 0.0f;

    while (window.IsOpen()) {
        window.PollEvents();

        Time::Update();
        const float dt = Time::DeltaTime();
        fixedAcc += dt;

        while (fixedAcc >= kFixedStep) {
            app.OnFixedUpdate();
            fixedAcc -= kFixedStep;
        }

        app.OnUpdate(dt);
        app.OnRender();
        InputManager::Get().EndFrame();
    }
}

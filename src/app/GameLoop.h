// 애플리케이션 콜백을 일정한 프레임 흐름으로 실행하는 게임 루프를 선언합니다.
#pragma once

class Application;

class GameLoop {
public:
    static constexpr float kFixedStep = 1.0f / 60.0f;

    static void Run(Application& app);
};

#pragma once

class Application;

class GameLoop {
public:
    static constexpr float kFixedStep = 1.0f / 60.0f;

    static void Run(Application& app);
};

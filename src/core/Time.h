#pragma once
#include <chrono>
#include <cstdint>

class Time {
public:
    static void Init();
    static void Update();

    static float DeltaTime()      { return s_deltaTime; }
    static float FixedDeltaTime() { return s_fixedDeltaTime; }
    static float TotalTime()      { return s_totalTime; }
    static uint64_t FrameCount()  { return s_frameCount; }

private:
    using Clock = std::chrono::high_resolution_clock;

    static Clock::time_point s_lastTime;
    static float s_deltaTime;
    static float s_fixedDeltaTime;
    static float s_totalTime;
    static uint64_t s_frameCount;
    static bool s_initialized;
};

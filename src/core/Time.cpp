// 프레임 시간과 누적 시간을 계산하는 전역 시간 상태를 구현합니다.
#include "core/Time.h"
#include <algorithm>

Time::Clock::time_point Time::s_lastTime{};
float Time::s_deltaTime = 0.0f;
float Time::s_fixedDeltaTime = 1.0f / 60.0f;
float Time::s_totalTime = 0.0f;
uint64_t Time::s_frameCount = 0;
bool Time::s_initialized = false;

void Time::Init() {
    s_lastTime = Clock::now();
    s_deltaTime = 0.0f;
    s_totalTime = 0.0f;
    s_frameCount = 0;
    s_initialized = true;
}

void Time::Update() {
    if (!s_initialized) {
        Init();
        return;
    }

    const auto now = Clock::now();
    const float rawDelta = std::chrono::duration<float>(now - s_lastTime).count();
    s_lastTime = now;

    s_deltaTime = std::min(rawDelta, 0.1f);
    s_totalTime += s_deltaTime;
    ++s_frameCount;
}

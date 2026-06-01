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

    // 디버깅 중 긴 정지나 창 이동으로 물리 루프가 폭주하지 않도록 델타타임을 제한한다.
    s_deltaTime = std::min(rawDelta, 0.1f);
    s_totalTime += s_deltaTime;
    ++s_frameCount;
}

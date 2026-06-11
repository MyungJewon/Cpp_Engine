// 간단한 누적 시간 타이머를 정의합니다.
#pragma once

class Timer {
public:
    void Update(float dt) {
        if (m_running) m_elapsed += dt;
    }

    void Start() { m_running = true; }
    void Stop() { m_running = false; }

    void Reset() {
        m_elapsed = 0.0f;
        m_running = false;
    }

    float Elapsed() const { return m_elapsed; }
    bool IsRunning() const { return m_running; }

private:
    float m_elapsed = 0.0f;
    bool m_running = false;
};

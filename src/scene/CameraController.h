// Orbit과 FPS 모드를 지원하는 카메라 컨트롤러를 선언합니다.
#pragma once

#include "math/Vec3.h"

class Camera;

enum class CameraMode {
    Orbit,
    FPS
};

class CameraController {
public:
    void Update(Camera& camera, float dt);
    void UpdateOrbit(Camera& camera, float dt);
    void UpdateFPS(Camera& camera, float dt);

    CameraMode mode = CameraMode::Orbit;
    Vec3 target = { 0.0f, 0.0f, 0.0f };
    float sensitivity = 0.01f;
    float zoomSpeed = 0.05f;
    float minDistance = 1.0f;
    float maxDistance = 20.0f;
    float moveSpeed = 4.0f;

private:
    float m_yaw = 0.0f;
    float m_pitch = 0.25f;
    float m_distance = 4.2f;
    float m_fpsYaw = 3.14159265358979323846f;
    float m_fpsPitch = 0.0f;
};

#include "scene/CameraController.h"
#include "input/InputManager.h"
#include "math/MathUtils.h"
#include "scene/Camera.h"
#include <cmath>

void CameraController::Update(Camera& camera, float dt) {
    if (mode == CameraMode::FPS) {
        UpdateFPS(camera, dt);
    } else {
        UpdateOrbit(camera, dt);
    }
}

void CameraController::UpdateOrbit(Camera& camera, float) {
    InputManager& input = InputManager::Get();

    // 좌클릭 드래그는 타깃을 중심으로 카메라의 방위각/고도각을 조절한다.
    if (input.IsMouseDown(MouseButton::Left)) {
        m_yaw -= input.MouseDX() * sensitivity;
        m_pitch += input.MouseDY() * sensitivity;
        m_pitch = Clamp(m_pitch, DegToRad(-85.0f), DegToRad(85.0f));
    }

    // 휠 입력은 타깃과 카메라 사이의 거리를 조절한다.
    m_distance -= input.ScrollDelta() * zoomSpeed;
    m_distance = Clamp(m_distance, minDistance, maxDistance);

    const float cp = std::cos(m_pitch);
    const float sp = std::sin(m_pitch);
    const float cy = std::cos(m_yaw);
    const float sy = std::sin(m_yaw);

    camera.target = target;
    camera.eye = {
        target.x + m_distance * cp * sy,
        target.y + m_distance * sp,
        target.z + m_distance * cp * cy
    };
    camera.up = { 0.0f, 1.0f, 0.0f };
}

void CameraController::UpdateFPS(Camera& camera, float dt) {
    InputManager& input = InputManager::Get();

    // 우클릭 드래그는 FPS 시점의 방위각/고도각을 조절한다.
    if (input.IsMouseDown(MouseButton::Right)) {
        m_fpsYaw -= input.MouseDX() * sensitivity;
        m_fpsPitch += input.MouseDY() * sensitivity;
        m_fpsPitch = Clamp(m_fpsPitch, DegToRad(-85.0f), DegToRad(85.0f));
    }

    const float cp = std::cos(m_fpsPitch);
    const float sp = std::sin(m_fpsPitch);
    const float cy = std::cos(m_fpsYaw);
    const float sy = std::sin(m_fpsYaw);

    const Vec3 forward = Vec3(cp * sy, sp, cp * cy).normalized();
    const Vec3 worldUp = { 0.0f, 1.0f, 0.0f };
    const Vec3 right = forward.cross(worldUp).normalized();

    Vec3 move;
    if (input.IsKeyDown(KeyCode::W)) move += forward;
    if (input.IsKeyDown(KeyCode::S)) move += -forward;
    if (input.IsKeyDown(KeyCode::D)) move += right;
    if (input.IsKeyDown(KeyCode::A)) move += -right;
    if (input.IsKeyDown(KeyCode::E)) move += worldUp;
    if (input.IsKeyDown(KeyCode::Q)) move += -worldUp;

    if (move.length() > 0.0f) {
        camera.eye += move.normalized() * (moveSpeed * dt);
    }

    camera.target = camera.eye + forward;
    camera.up = worldUp;
}

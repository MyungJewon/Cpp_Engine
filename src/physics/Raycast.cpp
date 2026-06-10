#include "physics/Raycast.h"

#include "ecs/Registry.hpp"
#include "math/Mat4.h"
#include "math/Vec4.h"
#include "physics/Collider.h"
#include "scene/Camera.h"
#include "scene/Transform.h"

#include <cmath>
#include <limits>

namespace {

constexpr float kRayEpsilon = 0.001f;
constexpr float kParallelEpsilon = 0.000001f;

Mat4 Inverse(const Mat4& mat) {
    const float* m = &mat.m[0][0];
    float inv[16];

    inv[0] = m[5] * m[10] * m[15] -
             m[5] * m[11] * m[14] -
             m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] -
             m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
              m[4] * m[11] * m[14] +
              m[8] * m[6] * m[15] -
              m[8] * m[7] * m[14] -
              m[12] * m[6] * m[11] +
              m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
             m[4] * m[11] * m[13] -
             m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
               m[4] * m[10] * m[13] +
               m[8] * m[5] * m[14] -
               m[8] * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
              m[1] * m[11] * m[14] +
              m[9] * m[2] * m[15] -
              m[9] * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
             m[0] * m[11] * m[14] -
             m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
              m[0] * m[11] * m[13] +
              m[8] * m[1] * m[15] -
              m[8] * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
              m[0] * m[10] * m[13] -
              m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
             m[1] * m[7] * m[14] -
             m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
              m[0] * m[7] * m[14] +
              m[4] * m[2] * m[15] -
              m[4] * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
              m[0] * m[7] * m[13] -
              m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
               m[0] * m[6] * m[13] +
               m[4] * m[1] * m[14] -
               m[4] * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (std::fabs(det) < kParallelEpsilon) {
        return Mat4::Identity();
    }

    det = 1.0f / det;

    Mat4 out;
    float* outM = &out.m[0][0];
    for (int i = 0; i < 16; ++i) {
        outM[i] = inv[i] * det;
    }

    return out;
}

bool IntersectSphere(const Ray& ray, const Vec3& center, float radius, float& distance, Vec3& normal) {
    Vec3 oc = ray.origin - center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f || std::fabs(a) < kParallelEpsilon) {
        return false;
    }

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t <= kRayEpsilon) {
        return false;
    }

    distance = t;
    Vec3 point = ray.origin + ray.direction * t;
    normal = (point - center).normalized();
    return true;
}

bool UpdateSlab(float origin, float direction, float minValue, float maxValue, int axis, float& tEnter, float& tExit, int& normalAxis) {
    if (std::fabs(direction) < kParallelEpsilon) {
        return origin >= minValue && origin <= maxValue;
    }

    float t1 = (minValue - origin) / direction;
    float t2 = (maxValue - origin) / direction;

    if (t1 > t2) {
        float temp = t1;
        t1 = t2;
        t2 = temp;
    }

    if (t1 > tEnter) {
        tEnter = t1;
        normalAxis = axis;
    }
    if (t2 < tExit) {
        tExit = t2;
    }

    return tEnter <= tExit;
}

bool IntersectAABB(const Ray& ray, const Vec3& boxMin, const Vec3& boxMax, float& distance, Vec3& normal) {
    float tEnter = -std::numeric_limits<float>::infinity();
    float tExit = std::numeric_limits<float>::infinity();
    int normalAxis = -1;

    if (!UpdateSlab(ray.origin.x, ray.direction.x, boxMin.x, boxMax.x, 0, tEnter, tExit, normalAxis) ||
        !UpdateSlab(ray.origin.y, ray.direction.y, boxMin.y, boxMax.y, 1, tEnter, tExit, normalAxis) ||
        !UpdateSlab(ray.origin.z, ray.direction.z, boxMin.z, boxMax.z, 2, tEnter, tExit, normalAxis)) {
        return false;
    }

    if (tEnter > tExit || tExit <= kRayEpsilon) {
        return false;
    }

    distance = tEnter > kRayEpsilon ? tEnter : tExit;

    normal = {};
    if (normalAxis >= 0 && tEnter > kRayEpsilon) {
        if (normalAxis == 0) normal.x = ray.direction.x > 0.0f ? -1.0f : 1.0f;
        if (normalAxis == 1) normal.y = ray.direction.y > 0.0f ? -1.0f : 1.0f;
        if (normalAxis == 2) normal.z = ray.direction.z > 0.0f ? -1.0f : 1.0f;
    }

    return true;
}

} // namespace

RaycastHit Raycast::Cast(const Ray& ray, Registry& reg) {
    RaycastHit closest;
    float closestDistance = std::numeric_limits<float>::max();

    auto& colliders = reg.pool<Collider>();
    for (size_t i = 0; i < colliders.size(); ++i) {
        Entity entity = colliders.entity_at(i);
        Transform* transform = reg.try_get<Transform>(entity);
        if (!transform) {
            continue;
        }

        const Collider& collider = colliders.get(entity);
        const Mat4& worldMatrix = transform->worldMatrix;
        Vec3 worldPosition = {
            worldMatrix.m[0][3],
            worldMatrix.m[1][3],
            worldMatrix.m[2][3],
        };
        Vec3 center = worldPosition + collider.center;

        float distance = 0.0f;
        Vec3 normal;
        bool hit = false;

        if (collider.shape == ColliderShape::Sphere) {
            hit = IntersectSphere(ray, center, collider.radius, distance, normal);
        } else if (collider.shape == ColliderShape::AABB) {
            Vec3 boxMin = center - collider.halfExtents;
            Vec3 boxMax = center + collider.halfExtents;
            hit = IntersectAABB(ray, boxMin, boxMax, distance, normal);
        }

        if (hit && distance < closestDistance) {
            closestDistance = distance;
            closest.entity = entity;
            closest.distance = distance;
            closest.point = ray.origin + ray.direction * distance;
            closest.normal = normal;
            closest.hit = true;
        }
    }

    return closest;
}

Ray Raycast::ScreenPointToRay(float screenX, float screenY, int screenW, int screenH, const Camera& camera) {
    float ndcX = (2.0f * screenX / static_cast<float>(screenW)) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / static_cast<float>(screenH));

    Mat4 invProj = Inverse(camera.GetProjection());
    Vec4 viewDir = invProj * Vec4(ndcX, ndcY, -1.0f, 1.0f);
    viewDir.z = -1.0f;
    viewDir.w = 0.0f;

    Mat4 invView = Inverse(camera.GetView());
    Vec3 worldDir = (invView * viewDir).xyz().normalized();

    Ray ray;
    ray.origin = camera.eye;
    ray.direction = worldDir;
    return ray;
}

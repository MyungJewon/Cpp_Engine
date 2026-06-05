#pragma once
#include "math/Vec3.h"
#include "resource/ObjLoader.h"
#include <algorithm>
#include <cfloat>

enum class ColliderShape { AABB, Sphere };

struct Collider {
    ColliderShape shape       = ColliderShape::AABB;
    Vec3          center      = {0, 0, 0};
    Vec3          halfExtents = {0.5f, 0.5f, 0.5f};
    float         radius      = 0.5f;
    float         restitution = 0.0f;  // 0 = no bounce, 1 = perfectly elastic
    float         friction    = 0.4f;  // 0 = no friction, 1 = stop tangential velocity
    bool          isTrigger   = false;

    static Collider FromMesh(const Mesh& mesh, const Vec3& scale = {1.0f, 1.0f, 1.0f}) {
        Vec3 minV = { FLT_MAX,  FLT_MAX,  FLT_MAX};
        Vec3 maxV = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (const auto& v : mesh.vertices) {
            minV.x = std::min(minV.x, v.pos.x);
            minV.y = std::min(minV.y, v.pos.y);
            minV.z = std::min(minV.z, v.pos.z);
            maxV.x = std::max(maxV.x, v.pos.x);
            maxV.y = std::max(maxV.y, v.pos.y);
            maxV.z = std::max(maxV.z, v.pos.z);
        }

        Collider col;
        col.center.x      = (minV.x + maxV.x) * 0.5f * scale.x;
        col.center.y      = (minV.y + maxV.y) * 0.5f * scale.y;
        col.center.z      = (minV.z + maxV.z) * 0.5f * scale.z;
        col.halfExtents.x = (maxV.x - minV.x) * 0.5f * scale.x;
        col.halfExtents.y = (maxV.y - minV.y) * 0.5f * scale.y;
        col.halfExtents.z = (maxV.z - minV.z) * 0.5f * scale.z;
        return col;
    }
};

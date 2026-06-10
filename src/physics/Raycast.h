#pragma once

#include "physics/Ray.h"
#include "physics/RaycastHit.h"

class Camera;
class Registry;

class Raycast {
public:
    static RaycastHit Cast(const Ray& ray, Registry& reg);
    static Ray ScreenPointToRay(float screenX, float screenY, int screenW, int screenH, const Camera& camera);
};

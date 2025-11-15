#pragma once
#include "Ray.h"
#include "AABB.h"

namespace Raycast {
    bool RayIntersectsAABB(const Ray& ray, const AABB& box, float& outT);
}

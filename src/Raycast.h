#pragma once
#include "Ray.h"
#include "AABB.h"

namespace Raycast {
    bool Raycast::RayIntersectsAABB(const Ray& ray, const AABB& box, float& outT);
}

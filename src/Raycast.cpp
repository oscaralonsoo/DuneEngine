#include "Raycast.h"
#include <algorithm>

bool Raycast::RayIntersectsAABB(const Ray& ray, const AABB& box, float& outT)
{
    float tNear = 0.0f;
    float tFar  = std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; axis++) {
        float inverseDirection = 1.0f / ray.direction[axis];

        float t1 = (box.min[axis] - ray.origin[axis]) * inverseDirection;
        float t2 = (box.max[axis] - ray.origin[axis]) * inverseDirection;

        if (inverseDirection < 0.0f) std::swap(t1, t2);

        tNear = std::max(tNear, t1);
        tFar  = std::min(tFar, t2);

        if (tFar < tNear)
            return false;
    }

    outT = tNear;
    return true;
}


#pragma once
#include <glm/glm.hpp>
#include <limits>

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;

    AABB()
        : min(glm::vec3( std::numeric_limits<float>::max())),
          max(glm::vec3(-std::numeric_limits<float>::max())) {}

    AABB(const glm::vec3& min, const glm::vec3& max)
        : min(min), max(max) {}
};

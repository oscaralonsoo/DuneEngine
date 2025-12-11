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

    void Reset()
    {
        min = glm::vec3( std::numeric_limits<float>::max());
        max = glm::vec3(-std::numeric_limits<float>::max());
    }

    void Encapsulate(const glm::vec3& p)
    {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    void Encapsulate(const AABB& other)
    {
        Encapsulate(other.min);
        Encapsulate(other.max);
    }

    glm::vec3 GetCenter() const  { return (min + max) * 0.5f; }
    glm::vec3 GetExtent() const  { return (max - min) * 0.5f; }

    void GetVertices(glm::vec3 out[8]) const
    {
        out[0] = glm::vec3(min.x, min.y, min.z);
        out[1] = glm::vec3(max.x, min.y, min.z);
        out[2] = glm::vec3(min.x, max.y, min.z);
        out[3] = glm::vec3(max.x, max.y, min.z);
        out[4] = glm::vec3(min.x, min.y, max.z);
        out[5] = glm::vec3(max.x, min.y, max.z);
        out[6] = glm::vec3(min.x, max.y, max.z);
        out[7] = glm::vec3(max.x, max.y, max.z);
    }
};

inline AABB TransformAABB(const AABB& box, const glm::mat4& transform)
{
    glm::vec3 corners[8];
    box.GetVertices(corners);

    AABB result;

    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 worldPos = transform * glm::vec4(corners[i], 1.0f);
        result.Encapsulate(glm::vec3(worldPos));
    }

    return result;
}

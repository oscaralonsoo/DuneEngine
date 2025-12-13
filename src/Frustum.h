#pragma once

#include <glm/glm.hpp>
#include "AABB.h"

class Frustum
{
public:
    enum PlaneIndex
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count
    };

    Frustum() = default;

    // Llamar cada frame con la view y projection de la cámara
    void Update(const glm::mat4& view, const glm::mat4& projection);

    // Devuelve true si la caja está (parcial o totalmente) dentro del frustum
    bool ContainsAABB(const AABB& box) const;

private:
    struct Plane
    {
        glm::vec3 normal = glm::vec3(0.0f);
        float d = 0.0f;

        float Distance(const glm::vec3& p) const
        {
            return glm::dot(normal, p) + d;
        }
    };

    Plane mPlanes[Count];
};
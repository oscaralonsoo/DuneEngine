#pragma once
#include <glm/glm.hpp>
#include "Ray.h"

class MousePicker
{
public:
    static Ray ScreenPointToRay(
        float mouseX, float mouseY,
        float viewportWidth, float viewportHeight,
        const glm::mat4& view,
        const glm::mat4& projection
    );
};

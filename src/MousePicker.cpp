#include "MousePicker.h"

Ray MousePicker::ScreenPointToRay(
    float mouseX, float mouseY,
    float viewportWidth, float viewportHeight,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    float x = (2.0f * mouseX) / viewportWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / viewportHeight;

    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 rayWorld =
        glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

    glm::vec3 origin =
        glm::vec3(glm::inverse(view)[3]);

    return Ray{ origin, rayWorld };
}

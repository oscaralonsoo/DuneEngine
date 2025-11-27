#include "TransformComponent.h"
#include "GameObject.h"
#include <glm/gtc/matrix_transform.hpp>

TransformComponent::TransformComponent(GameObject* owner)
    : Component(ComponentType::Transform, owner)
{
    UpdateWorldMatrix();
}

void TransformComponent::UpdateWorldMatrix()
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

    mWorldMatrix = translation * rotationZ * rotationY * rotationX * scaling;
}

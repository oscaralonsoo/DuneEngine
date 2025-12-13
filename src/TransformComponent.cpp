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
    // Usar los getters para no confundir IntelliSense
    const glm::vec3& pos = GetPosition();
    const glm::vec3& rot = GetRotation();
    const glm::vec3& scl = GetScale();

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 rotationX   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.x), glm::vec3(1, 0, 0));
    glm::mat4 rotationY   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.y), glm::vec3(0, 1, 0));
    glm::mat4 rotationZ   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.z), glm::vec3(0, 0, 1));
    glm::mat4 scaling     = glm::scale(glm::mat4(1.0f), scl);

    // Local matrix
    glm::mat4 localMatrix = translation * rotationZ * rotationY * rotationX * scaling;

    // If has parent, multiply by parent's world matrix
    if (auto parent = GetOwner()->GetParent())
    {
        if (auto parentTransform = parent->GetComponent<TransformComponent>())
        {
            mWorldMatrix = parentTransform->GetWorldTransform() * localMatrix;
        }
        else
        {
            mWorldMatrix = localMatrix;
        }
    }
    else
    {
        mWorldMatrix = localMatrix;
    }

    // Update children
    for (auto& child : GetOwner()->GetChildren())
    {
        if (auto childTransform = child->GetComponent<TransformComponent>())
        {
            childTransform->UpdateWorldMatrix();
        }
    }
}


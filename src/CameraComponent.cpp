#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include <glm/gtc/matrix_inverse.hpp>

CameraComponent::CameraComponent(GameObject* owner)
    : Component(ComponentType::Camera, owner)
{}

bool CameraComponent::Update()
{
    UpdateFromTransform();
    return true;
}

void CameraComponent::UpdateFromTransform()
{
    auto* owner = GetOwner();
    if (!owner)
        return;

    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform)
        return;

    const glm::mat4& world = transform->GetWorldTransform();

    mViewMatrix = glm::inverse(world);
    
    mPosition = glm::vec3(world[3]);
}

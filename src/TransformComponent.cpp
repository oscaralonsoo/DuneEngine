#include "TransformComponent.h"
#include "GameObject.h"

TransformComponent::TransformComponent(GameObject* owner)
    : Component(ComponentType::Transform, owner)
{
}

#include "GameObject.h"

GameObject::GameObject()
{
    mName = "GameObject";
}

Component &GameObject::CreateComponent(ComponentType type)
{
    std::unique_ptr<Component> component;

    switch (type)
    {
    case ComponentType::Transform:
        // component = std::make_unique<ComponentTransform>(this);
        break;
    case ComponentType::Mesh:
        // component = std::make_unique<ComponentMesh>(this);
        break;
    case ComponentType::Material:
        // component = std::make_unique<ComponentMaterial>(this);
        break;
    default:
        component = std::make_unique<Component>(type, this);
        break;
    }

    mComponents.push_back(std::move(component));

    return *component;
}

void GameObject::SetName(const std::string &name)
{
    mName = name;
}
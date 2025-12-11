#include "GameObject.h"
#include "MeshComponent.h"
#include "PrimitiveMesh.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"

GameObject::GameObject()
{
    mName = "GameObject";
    CreateComponent(ComponentType::Transform);
}

Component &GameObject::CreateComponent(ComponentType type)
{
    std::unique_ptr<Component> component;

    switch (type)
    {
    case ComponentType::Transform:
        component = std::make_unique<TransformComponent>(this);
        break;
    case ComponentType::Mesh:
        component = std::make_unique<MeshComponent>(this);
        break;
    case ComponentType::Material:
        component = std::make_unique<MaterialComponent>(this);
        break;
    case ComponentType::Camera:
        component = std::make_unique<CameraComponent>(this);
        break;
    default:
        component = std::make_unique<Component>(type, this);
        break;
    }

    mComponents.push_back(std::move(component));

    return *mComponents.back();
}

void GameObject::SetName(const std::string &name)
{
    mName = name;
}

bool GameObject::Update()
{
    if (!mActive)
        return true;

    for (auto& comp : mComponents)
    {
        if (comp->IsActive())
        {
            if (!comp->Update())
                return false;
        }
    }

    return true;
}

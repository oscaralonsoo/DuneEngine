#include "GameObject.h"
#include "MeshComponent.h"
#include "PrimitiveMesh.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"

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

// Parent-Child hierarchy implementation
void GameObject::SetParent(std::shared_ptr<GameObject> parent)
{
    if (auto currentParent = mParent.lock())
    {
        currentParent->RemoveChild(shared_from_this());
    }

    mParent = parent;

    if (parent)
    {
        parent->AddChild(shared_from_this());
    }
}

void GameObject::AddChild(std::shared_ptr<GameObject> child)
{
    if (child && std::find(mChildren.begin(), mChildren.end(), child) == mChildren.end())
    {
        mChildren.push_back(child);
        child->mParent = shared_from_this();
    }
}

void GameObject::RemoveChild(std::shared_ptr<GameObject> child)
{
    if (!child) return;

    auto it = std::find(mChildren.begin(), mChildren.end(), child);
    if (it != mChildren.end())
    {
        mChildren.erase(it);
        child->mParent.reset();
        OnChildRemoved(child);
    }
}

std::vector<std::shared_ptr<GameObject>> GameObject::GetAllDescendants() const
{
    std::vector<std::shared_ptr<GameObject>> descendants;

    for (const auto& child : mChildren)
    {
        descendants.push_back(child);
        auto childDescendants = child->GetAllDescendants();
        descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
    }

    return descendants;
}

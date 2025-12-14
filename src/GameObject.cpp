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

void GameObject::InsertChildAt(std::shared_ptr<GameObject> child, size_t index)
{
    if (!child) return;

    // Remove from current parent if it has one
    if (auto currentParent = child->mParent.lock())
    {
        currentParent->RemoveChild(child);
    }

    // Clamp index to valid range
    if (index > mChildren.size())
    {
        index = mChildren.size();
    }

    // Insert at the specified position
    mChildren.insert(mChildren.begin() + index, child);
    child->mParent = shared_from_this();
}

size_t GameObject::GetChildIndex(std::shared_ptr<GameObject> child) const
{
    auto it = std::find(mChildren.begin(), mChildren.end(), child);
    if (it != mChildren.end())
    {
        return std::distance(mChildren.begin(), it);
    }
    return static_cast<size_t>(-1);
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

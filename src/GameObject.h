#pragma once
#include <string>
#include <vector>
#include "Component.h"
#include <memory>

class Component;

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
    GameObject();
    ~GameObject() = default;

    Component &CreateComponent(ComponentType type);

    const std::string GetName() const { return mName; }
    void SetName(const std::string &name);

    void SetSelected(bool selected) { mSelected = selected; }
    bool IsSelected() const { return mSelected; }

    template <typename T>
    T* GetComponent()
    {
        for (auto& comp : mComponents)
        {
            if (auto ptr = dynamic_cast<T*>(comp.get()))
                return ptr;
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<Component>>& GetComponents() const { return mComponents; }

    // Parent-Child hierarchy
    void SetParent(std::shared_ptr<GameObject> parent);
    std::shared_ptr<GameObject> GetParent() const { return mParent.lock(); }
    void AddChild(std::shared_ptr<GameObject> child);
    void RemoveChild(std::shared_ptr<GameObject> child);
    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return mChildren; }

    // Get all descendants (recursive)
    std::vector<std::shared_ptr<GameObject>> GetAllDescendants() const;

private:
    bool mActive = true;
    std::string mName;
    std::vector<std::unique_ptr<Component>> mComponents;
    bool mSelected = false;

    // Parent-Child hierarchy
    std::weak_ptr<GameObject> mParent;
    std::vector<std::shared_ptr<GameObject>> mChildren;
};

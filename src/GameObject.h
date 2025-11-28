#pragma once
#include <string>
#include <vector>
#include "Component.h"
#include <memory>

class Component;

class GameObject
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

private:
    bool mActive = true;
    std::string mName;
    std::vector<std::unique_ptr<Component>> mComponents;

    bool mSelected = false;
};

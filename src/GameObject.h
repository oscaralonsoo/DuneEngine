#pragma once
#include <string>
#include <vector>
#include "Component.h"
#include <memory>

class GameObject
{
public:
    GameObject();
    ~GameObject() = default;

    Component &CreateComponent(ComponentType type);

    const std::string GetName() const { return mName; }
    void SetName(const std::string& name);

private:
    bool mActive = true;
    std::string mName;
    std::vector<std::unique_ptr<Component>> mComponents;
};

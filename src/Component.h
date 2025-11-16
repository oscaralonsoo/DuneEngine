#pragma once

enum class ComponentType
{
    Transform,
    Mesh,
    Material,
    Camera
};

class GameObject;

class Component
{
public:
    Component(ComponentType type, GameObject* owner);
    ~Component() = default;

    virtual bool Update();

    virtual void Enable();
    virtual void Disable();

private:
    ComponentType mType;
    bool mActive = true;
    GameObject* mOwner;
};

#pragma once
#include "GameObject.h"
#include "ComponentTypes.h" 
class GameObject; 

class Component
{
public:
    Component(ComponentType type, GameObject* owner);
    ~Component() = default;

    virtual bool Update();

    virtual void Enable();
    virtual void Disable();

    ComponentType GetType() const { return mType; }
    GameObject* GetOwner() const { return mOwner; }

    virtual void OnInspectorRender(float panelWidth) {}

    bool IsActive() const { return mActive; }
    
protected:
    GameObject* GetOwner() const { return mOwner; }

private:
    ComponentType mType;
    bool mActive = true;
    GameObject* mOwner;
};

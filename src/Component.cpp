#include "Component.h"
#include "GameObject.h"

Component::Component(ComponentType type, GameObject *owner)
    : mType(type), mOwner(owner) {}

bool Component::Update()
{
    return true;
}

void Component::Enable()
{
    mActive = true;
}

void Component::Disable()
{
    mActive = false;
}
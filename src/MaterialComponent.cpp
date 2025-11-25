#include "MaterialComponent.h"

MaterialComponent::MaterialComponent(GameObject *owner, const std::shared_ptr<Material> &material)
    : Component(ComponentType::Material, owner), mMaterial(material)
{
}

void MaterialComponent::SetMaterial(const std::shared_ptr<Material> &material)
{
    mMaterial = material;
}

const std::shared_ptr<Material> &MaterialComponent::GetMaterial() const
{
    return mMaterial;
}

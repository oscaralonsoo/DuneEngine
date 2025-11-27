#pragma once

#include "Material.h"
#include "Component.h"

class MaterialComponent : public Component
{
public:
    MaterialComponent(GameObject *owner, const std::shared_ptr<Material> &material = nullptr);
    ~MaterialComponent() = default;

    void SetMaterial(const std::shared_ptr<Material> &material);
    const std::shared_ptr<Material> &GetMaterial() const;

    void OnInspectorRender(float panelWidth);

private:
    std::shared_ptr<Material> mMaterial;
};
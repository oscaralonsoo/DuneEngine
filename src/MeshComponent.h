#pragma once
#include "Component.h"
#include "Material.h"
#include <memory>

class Mesh;

class MeshComponent : public Component
{
public:
    MeshComponent(GameObject *owner, const std::shared_ptr<Mesh> &mesh = nullptr);

    ~MeshComponent() = default;

    void SetMesh(const std::shared_ptr<Mesh> &mesh);
    const std::shared_ptr<Mesh> &GetMesh() const;

private:
    std::shared_ptr<Mesh> mMesh;
};

#include "MeshComponent.h"
#include "Mesh.h"
#include "Globals.h"

MeshComponent::MeshComponent(GameObject *owner, const std::shared_ptr<Mesh> &mesh)
    : Component(ComponentType::Mesh, owner), mMesh(mesh)
{
}

void MeshComponent::SetMesh(const std::shared_ptr<Mesh> &mesh)
{
    mMesh = mesh;
}

const std::shared_ptr<Mesh> &MeshComponent::GetMesh() const
{
    return mMesh;
}

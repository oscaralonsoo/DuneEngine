#include "MeshComponent.h"
#include "Globals.h"

MeshComponent::MeshComponent(GameObject *owner, const std::shared_ptr<Mesh> &mesh)
    : Component(ComponentType::Mesh, owner), mMesh(mesh)
{
}

bool MeshComponent::Update()
{
    LOG_INFO("Updating MeshComponent");
    return true;
}

void MeshComponent::SetMesh(const std::shared_ptr<Mesh> &mesh)
{
    mMesh = mesh;
}

const std::shared_ptr<Mesh> &MeshComponent::GetMesh() const
{
    return mMesh;
}
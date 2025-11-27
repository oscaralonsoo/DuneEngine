#include "MeshComponent.h"
#include "Mesh.h"
#include "Globals.h"
#include <imgui.h>

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

void MeshComponent::OnInspectorRender(float panelWidth)
{
    if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mMesh)
        {
            ImGui::Text("Name: %s", mMesh->GetName().c_str());
        }
        else
        {
            ImGui::TextDisabled("No Mesh Assigned");
        }
    }
}

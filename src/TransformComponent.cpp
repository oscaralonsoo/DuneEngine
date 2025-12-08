#include "TransformComponent.h"
#include "GameObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"

TransformComponent::TransformComponent(GameObject* owner)
    : Component(ComponentType::Transform, owner)
{
    UpdateWorldMatrix();
}

void TransformComponent::UpdateWorldMatrix()
{
    // Usar los getters para no confundir IntelliSense
    const glm::vec3& pos = GetPosition();
    const glm::vec3& rot = GetRotation();
    const glm::vec3& scl = GetScale();

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 rotationX   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.x), glm::vec3(1, 0, 0));
    glm::mat4 rotationY   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.y), glm::vec3(0, 1, 0));
    glm::mat4 rotationZ   = glm::rotate(glm::mat4(1.0f), glm::radians(rot.z), glm::vec3(0, 0, 1));
    glm::mat4 scaling     = glm::scale(glm::mat4(1.0f), scl);

    // Local matrix
    glm::mat4 localMatrix = translation * rotationZ * rotationY * rotationX * scaling;

    // If has parent, multiply by parent's world matrix
    if (auto parent = GetOwner()->GetParent())
    {
        if (auto parentTransform = parent->GetComponent<TransformComponent>())
        {
            mWorldMatrix = parentTransform->GetWorldTransform() * localMatrix;
        }
        else
        {
            mWorldMatrix = localMatrix;
        }
    }
    else
    {
        mWorldMatrix = localMatrix;
    }

    // Update children
    for (auto& child : GetOwner()->GetChildren())
    {
        if (auto childTransform = child->GetComponent<TransformComponent>())
        {
            childTransform->UpdateWorldMatrix();
        }
    }
}

void TransformComponent::OnInspectorRender(float panelWidth)
{
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Tomar los valores en variables LOCALES con otro nombre para no sombrear
        glm::vec3 pos = GetPosition();
        glm::vec3 rot = GetRotation();
        glm::vec3 scl = GetScale();

        ImGui::PushItemWidth(panelWidth * 0.95f);

        if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
            SetPosition(pos);

        if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f))
            SetRotation(rot);

        if (ImGui::DragFloat3("Scale", &scl.x, 0.01f))
        {
            scl = glm::max(scl, glm::vec3(0.001f));
            SetScale(scl);
        }

        if (ImGui::Button("Reset Transform"))
        {
            SetPosition(glm::vec3(0.0f));
            SetRotation(glm::vec3(0.0f));
            SetScale(glm::vec3(1.0f));
        }

        ImGui::PopItemWidth();
    }
}

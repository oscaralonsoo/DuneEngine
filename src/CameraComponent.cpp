#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "imgui.h"
#include <glm/gtc/matrix_inverse.hpp>

CameraComponent::CameraComponent(GameObject* owner)
    : Component(ComponentType::Camera, owner)
{
    // Valores por defecto ya vienen en Camera
}

bool CameraComponent::Update()
{
    UpdateFromTransform();
    return true;
}

void CameraComponent::UpdateFromTransform()
{
    auto* owner = GetOwner();
    if (!owner)
        return;

    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform)
        return;

    const glm::mat4& world = transform->GetWorldTransform();

    // View = inverse(world)
    mViewMatrix = glm::inverse(world);

    // Posición = traducción del world
    mPosition = glm::vec3(world[3]);
}

void CameraComponent::OnInspectorRender(float panelWidth)
{
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushItemWidth(panelWidth * 0.95f);

        float fov       = GetFOV();
        float nearClip  = GetNearClip();
        float farClip   = GetFarClip();

        if (ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f))
            SetFOV(fov);

        if (ImGui::DragFloat("Near", &nearClip, 0.01f, 0.01f, farClip - 0.01f))
            SetNearClip(nearClip);

        if (ImGui::DragFloat("Far", &farClip, 1.0f, nearClip + 0.01f, 10000.0f))
            SetFarClip(farClip);

        ImGui::PopItemWidth();
    }
}
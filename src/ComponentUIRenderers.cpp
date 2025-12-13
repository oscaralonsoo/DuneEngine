#include "ComponentUIRenderers.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Engine.h"
#include "ModuleInput.h"
#include "FileDialogUtils.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <filesystem>

namespace ComponentUI
{
    // ============================================================================
    // TransformComponentUI
    // ============================================================================
    
    void TransformComponentUI::Render(TransformComponent* component, float panelWidth)
    {
        if (!component)
            return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Get current values
            glm::vec3 pos = component->GetPosition();
            glm::vec3 rot = component->GetRotation();
            glm::vec3 scl = component->GetScale();

            ImGui::PushItemWidth(panelWidth * 0.95f);

            if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                component->SetPosition(pos);

            if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f))
                component->SetRotation(rot);

            if (ImGui::DragFloat3("Scale", &scl.x, 0.01f))
            {
                scl = glm::max(scl, glm::vec3(0.001f));
                component->SetScale(scl);
            }

            if (ImGui::Button("Reset Transform"))
            {
                component->SetPosition(glm::vec3(0.0f));
                component->SetRotation(glm::vec3(0.0f));
                component->SetScale(glm::vec3(1.0f));
            }

            ImGui::PopItemWidth();
        }
    }

    // ============================================================================
    // MeshComponentUI
    // ============================================================================
    
    void MeshComponentUI::Render(MeshComponent* component, float panelWidth)
    {
        if (!component)
            return;

        if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& mesh = component->GetMesh();
            if (mesh)
            {
                ImGui::Text("Name: %s", mesh->GetName().c_str());
            }
            else
            {
                ImGui::TextDisabled("No Mesh Assigned");
            }
        }
    }

    // ============================================================================
    // MaterialComponentUI
    // ============================================================================
    
    void MaterialComponentUI::Render(MaterialComponent* component, float panelWidth)
    {
        if (!component)
            return;

        if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
            return;

        const auto& material = component->GetMaterial();
        if (!material)
        {
            ImGui::TextDisabled("No Material Assigned");
            return;
        }

        ImGui::Text("Name: %s", material->GetName().c_str());

        auto& props = material->GetProperties();
        ImGui::ColorEdit4("Color", &props.color.r);
        ImGui::SliderFloat("Alpha", &props.color.a, 0.0f, 1.0f);

        if (!ImGui::TreeNode("Textures"))
            return;

        auto& texture = material->GetTextures();
        auto input = Engine::GetInstance().input.get();

        DrawTextureSlot("Albedo", texture.albedo, input);
        DrawTextureSlot("Normal", texture.normal, input);
        DrawTextureSlot("Metallic", texture.metallic, input);
        DrawTextureSlot("Roughness", texture.roughness, input);
        DrawTextureSlot("AO", texture.ao, input);
        DrawTextureSlot("Emissive", texture.emissive, input);

        ImGui::TreePop();
    }

    void MaterialComponentUI::DrawTextureSlot(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input)
    {
        RenderSlotBorder(name, texture);
        HandleDragDrop(name, texture, input);
        HandleSlotClick(name, texture);
        HandleFileDrop(name, texture, input);

        // X button to remove texture
        if (texture)
        {
            ImGui::SameLine();
            if (ImGui::Button(("X##" + std::string(name)).c_str()))
                texture = nullptr;
        }

        ImGui::Spacing();
    }

    void MaterialComponentUI::RenderSlotBorder(const char* name, std::shared_ptr<Texture>& texture)
    {
        ImGui::Text("%s", name);

        ImVec2 slotSize(32, 32);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton(("##slot_" + std::string(name)).c_str(), slotSize);
        bool hovered = ImGui::IsItemHovered();

        // Border
        ImU32 borderColor = hovered ? IM_COL32(100, 149, 237, 255) : IM_COL32(255, 255, 255, 255);
        ImGui::GetWindowDrawList()->AddRect(
            pos,
            ImVec2(pos.x + slotSize.x, pos.y + slotSize.y),
            borderColor
        );

        // Draw texture preview inside the slot
        if (texture)
        {
            GLuint id = texture->GetID();
            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)(uintptr_t)id,
                pos,
                ImVec2(pos.x + slotSize.x, pos.y + slotSize.y)
            );
        }
    }

    void MaterialComponentUI::HandleSlotClick(const char* name, std::shared_ptr<Texture>& texture)
    {
        if (ImGui::IsItemClicked())
        {
            std::string path = FileDialogUtils::OpenImageFile();
            if (!path.empty())
                texture = std::make_shared<Texture>(path.c_str());
        }
    }

    void MaterialComponentUI::RenderTexturePreview(const char* name, std::shared_ptr<Texture>& texture)
    {
        if (!texture)
            return;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 slotSize(32, 32);

        GLuint id = texture->GetID();
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(uintptr_t)id,
            pos,
            ImVec2(pos.x + slotSize.x, pos.y + slotSize.y)
        );

        ImGui::SameLine();
        if (ImGui::Button(("X##" + std::string(name)).c_str()))
            texture = nullptr;
    }

    void MaterialComponentUI::HandleDragDrop(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PAYLOAD"))
            {
                const char* relativePath = (const char*)payload->Data;
                if (relativePath && relativePath[0] != '\0')
                {
                    std::filesystem::path fullPath = "Assets" / std::filesystem::path(relativePath);
                    texture = std::make_shared<Texture>(fullPath);
                }
            }
            else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
            {
                const char* path = (const char*)payload->Data;
                if (path && path[0] != '\0')
                    texture = std::make_shared<Texture>(path);
            }
            ImGui::EndDragDropTarget();
        }
    }

    void MaterialComponentUI::HandleFileDrop(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input)
    {
        bool hovered = ImGui::IsItemHovered();
        if (input->WasFileDropped() && hovered)
        {
            std::string path = input->GetDraggedFile();
            if (!path.empty())
            {
                texture = std::make_shared<Texture>(path.c_str());
                input->ClearDropState();
                input->ClearDraggedFile();
            }
        }
    }

    // ============================================================================
    // CameraComponentUI
    // ============================================================================
    
    void CameraComponentUI::Render(CameraComponent* component, float panelWidth)
    {
        if (!component)
            return;

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PushItemWidth(panelWidth * 0.95f);

            float fov = component->GetFOV();
            float nearClip = component->GetNearClip();
            float farClip = component->GetFarClip();

            if (ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f))
                component->SetFOV(fov);

            if (ImGui::DragFloat("Near", &nearClip, 0.01f, 0.01f, farClip - 0.01f))
                component->SetNearClip(nearClip);

            if (ImGui::DragFloat("Far", &farClip, 1.0f, nearClip + 0.01f, 10000.0f))
                component->SetFarClip(farClip);

            ImGui::PopItemWidth();
        }
    }
}

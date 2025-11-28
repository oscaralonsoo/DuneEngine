#include "MaterialComponent.h"
#include "Engine.h"
#include "ModuleInput.h"
#include <imgui.h>
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <SDL3/SDL.h>
#include <filesystem>

MaterialComponent::MaterialComponent(GameObject* owner, const std::shared_ptr<Material>& material)
    : Component(ComponentType::Material, owner), mMaterial(material)
{
}

void MaterialComponent::SetMaterial(const std::shared_ptr<Material>& material)
{
    mMaterial = material;
}

const std::shared_ptr<Material>& MaterialComponent::GetMaterial() const
{
    return mMaterial;
}

// Opens a file dialog for selecting image files
std::string OpenFile()
{
    OPENFILENAMEA openFileName;
    CHAR szFile[260] = { 0 };
    CHAR szFilter[] = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds\0All Files\0*.*\0";

    ZeroMemory(&openFileName, sizeof(openFileName));
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = nullptr;
    openFileName.lpstrFile = szFile;
    openFileName.nMaxFile = sizeof(szFile);
    openFileName.lpstrFilter = szFilter;
    openFileName.nFilterIndex = 1;
    openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&openFileName) == TRUE)
        return std::string(openFileName.lpstrFile);

    return "";
}

// Renders a texture slot in the inspector
void MaterialComponent::DrawTextureSlot(const char* name, std::shared_ptr<Texture>& texture, ModuleInput* input)
{
    ImGui::Text("%s", name);

    ImVec2 slotSize(32, 32);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(("##slot_" + std::string(name)).c_str(), slotSize);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // Border color: blue on hover, white otherwise
    ImU32 borderColor = hovered ? IM_COL32(100, 149, 237, 255) : IM_COL32(255, 255, 255, 255);

    // Draw slot border
    ImGui::GetWindowDrawList()->AddRect(
        pos,
        ImVec2(pos.x + slotSize.x, pos.y + slotSize.y),
        borderColor
    );

    // Handle click to open file
    if (clicked)
    {
        std::string path = OpenFile();
        if (!path.empty())
            texture = std::make_shared<Texture>(path.c_str());
    }

    // Render texture preview
    if (texture)
    {
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

    // Handle drag and drop
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            const char* path = (const char*)payload->Data;
            if (path && path[0] != '\0')
                texture = std::make_shared<Texture>(path);
        }
        ImGui::EndDragDropTarget();
    }

    // Handle file drop
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

    ImGui::Spacing();
}

void MaterialComponent::OnInspectorRender(float panelWidth)
{
    if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    if (!mMaterial)
    {
        ImGui::TextDisabled("No Material Assigned");
        return;
    }

    ImGui::Text("Name: %s", mMaterial->GetName().c_str());

    auto& props = mMaterial->GetProperties();
    ImGui::ColorEdit4("Color", &props.color.r);
    ImGui::SliderFloat("Alpha", &props.color.a, 0.0f, 1.0f);

    if (!ImGui::TreeNode("Textures"))
        return;

    auto& texture = mMaterial->GetTextures();
    auto input = Engine::GetInstance().input.get();

    DrawTextureSlot("Albedo", texture.albedo, input);
    DrawTextureSlot("Normal", texture.normal, input);
    DrawTextureSlot("Metallic", texture.metallic, input);
    DrawTextureSlot("Roughness", texture.roughness, input);
    DrawTextureSlot("AO", texture.ao, input);
    DrawTextureSlot("Emissive", texture.emissive, input);

    ImGui::TreePop();
}

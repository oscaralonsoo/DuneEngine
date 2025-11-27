#include "MaterialComponent.h"
#include <imgui.h>

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

void MaterialComponent::OnInspectorRender(float panelWidth)
{
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mMaterial)
        {
            ImGui::Text("Name: %s", mMaterial->GetName().c_str());

            // Color
            auto& properties = mMaterial->GetProperties();
            ImGui::ColorEdit4("Color", &properties.color.r, ImGuiColorEditFlags_None);

            // Transparency
            ImGui::SliderFloat("Alpha", &properties.color.a, 0.0f, 1.0f);

            // Textures
            if (ImGui::TreeNode("Textures"))
            {
                auto& textures = mMaterial->GetTextures();

                // Albedo Texture
                ImGui::Text("Albedo");
                ImGui::SameLine();
                if (textures.albedo)
                {
                    ImGui::Text("%s", textures.albedo->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##albedo"))
                    {
                        textures.albedo = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##albedo", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.albedo = std::make_shared<Texture>(path);
                            textures.albedo->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Normal Texture
                ImGui::Text("Normal");
                ImGui::SameLine();
                if (textures.normal)
                {
                    ImGui::Text("%s", textures.normal->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##normal"))
                    {
                        textures.normal = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##normal", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.normal = std::make_shared<Texture>(path);
                            textures.normal->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Metallic Texture
                ImGui::Text("Metallic");
                ImGui::SameLine();
                if (textures.metallic)
                {
                    ImGui::Text("%s", textures.metallic->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##metallic"))
                    {
                        textures.metallic = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##metallic", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.metallic = std::make_shared<Texture>(path);
                            textures.metallic->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Roughness Texture
                ImGui::Text("Roughness");
                ImGui::SameLine();
                if (textures.roughness)
                {
                    ImGui::Text("%s", textures.roughness->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##roughness"))
                    {
                        textures.roughness = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##roughness", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.roughness = std::make_shared<Texture>(path);
                            textures.roughness->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // AO Texture
                ImGui::Text("AO");
                ImGui::SameLine();
                if (textures.ao)
                {
                    ImGui::Text("%s", textures.ao->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##ao"))
                    {
                        textures.ao = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##ao", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.ao = std::make_shared<Texture>(path);
                            textures.ao->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                // Emissive Texture
                ImGui::Text("Emissive");
                ImGui::SameLine();
                if (textures.emissive)
                {
                    ImGui::Text("%s", textures.emissive->GetName().c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("X##emissive"))
                    {
                        textures.emissive = nullptr;
                    }
                }
                else
                {
                    ImGui::Button("Drop##emissive", ImVec2(60, 20));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
                        {
                            const char* path = (const char*)payload->Data;
                            textures.emissive = std::make_shared<Texture>(path);
                            textures.emissive->SetName(std::filesystem::path(path).filename().string());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::TextDisabled("No Material Assigned");
        }
    }
}

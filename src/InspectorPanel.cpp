#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "CameraComponent.h"
#include "ComponentUIRenderers.h"
#include "ResourceUIRenderer.h"
#include <imgui.h>
#include <algorithm>

bool InspectorPanel::Start()
{
    return true;
}

void InspectorPanel::OnImGuiRender()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Inspector", nullptr, flags);

    if (mSelectedResource)
    {
        RenderResourceDetails();
    }
    else
    {
        // Check for GameObject selection
        auto scene = Engine::GetInstance().scene;
        if (scene)
        {
            std::shared_ptr<GameObject> selected = scene->GetSelected();
            if (selected)
            {
                static char nameBuffer[256];
                strcpy(nameBuffer, selected->GetName().c_str());
                ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    selected->SetName(nameBuffer);
                }
                ImGui::Separator();

                const auto& components = selected->GetComponents();
                float panelWidth = ImGui::GetContentRegionAvail().x;
                
                for (const auto& component : components)
                {
                    // Use the appropriate UI renderer
                    switch (component->GetType())
                    {
                        case ComponentType::Transform:
                            ComponentUI::TransformComponentUI::Render(
                                static_cast<TransformComponent*>(component.get()), 
                                panelWidth
                            );
                            break;
                        
                        case ComponentType::Mesh:
                            ComponentUI::MeshComponentUI::Render(
                                static_cast<MeshComponent*>(component.get()), 
                                panelWidth
                            );
                            break;
                        
                        case ComponentType::Material:
                            ComponentUI::MaterialComponentUI::Render(
                                static_cast<MaterialComponent*>(component.get()), 
                                panelWidth
                            );
                            break;
                        
                        case ComponentType::Camera:
                            ComponentUI::CameraComponentUI::Render(
                                static_cast<CameraComponent*>(component.get()), 
                                panelWidth
                            );
                            break;
                        
                        default:
                            component->OnInspectorRender(panelWidth);
                            break;
                    }
                    
                    ImGui::Separator();
                }
            }
            else
            {
                ImGui::TextDisabled("No selection");
            }
        }
    }

    ImGui::End();
}

void InspectorPanel::CleanUp()
{
    mSelectedResource.reset();
}

void InspectorPanel::SetSelectedResource(std::shared_ptr<Resource> resource, const std::filesystem::path& assetPath)
{
    mSelectedResource = resource;
    mSelectedResourcePath = assetPath;
}

void InspectorPanel::ClearSelectedResource()
{
    mSelectedResource.reset();
    mSelectedResourcePath.clear();
}

void InspectorPanel::RenderResourceDetails()
{
    if (!mSelectedResource)
        return;
    
    ResourceUI::RenderResourceDetails(mSelectedResource, mSelectedResourcePath);
}

#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include <imgui.h>
#include <algorithm>

bool InspectorPanel::Start()
{
    return true;
}

void InspectorPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    // Size & position
    float panelWidth  = workSize.x * InspectorPanel::kDefaultFraction;
    float panelHeight = workSize.y;
    float maxAllowed = workSize.x - (workSize.x * InspectorPanel::kDefaultFraction) - InspectorPanel::kMinCenterWidth;
    panelWidth = std::clamp(panelWidth, InspectorPanel::kMinPanelWidth, maxAllowed);

    // Anchor to right
    ImVec2 pos = ImVec2(workPos.x + workSize.x - panelWidth, workPos.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Inspector", nullptr, flags);

    auto scene = Engine::GetInstance().scene;
    if (scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        if (selected)
        {
            ImGui::Text("Name: %s", selected->GetName().c_str());
            ImGui::Separator();

            // Read transform from TransformComponent
            if (auto transformComponent = selected->GetComponent<TransformComponent>())
            {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        glm::vec3 position = transformComponent->GetPosition();
                        glm::vec3 scale    = transformComponent->GetScale();
                        glm::vec3 rotation = transformComponent->GetRotation();

                        ImGui::PushItemWidth(panelWidth * 0.95f);
                        if (ImGui::DragFloat3("##Position", &position.x, 0.1f))
                        transformComponent->SetPosition(position);
                        
                        ImGui::Text("Rotation");
                        if (ImGui::DragFloat3("##Rotation", &rotation.x, 0.1f))
                        transformComponent->SetRotation(rotation);
                        
                        ImGui::Text("Scale");
                        if (ImGui::DragFloat3("##Scale", &scale.x, 0.01f))
                        {
                            scale = glm::max(scale, glm::vec3(0.001f));
                            transformComponent->SetScale(scale);
                        }
                        
                        if (ImGui::Button("Reset Transform"))
                        transformComponent->SetPosition({0,0,0}),
                        transformComponent->SetRotation({0,0,0}),
                        transformComponent->SetScale({1,1,1});
                        
                        ImGui::PopItemWidth();
                    }
                    ImGui::Text("Position");
                }
            else
            {
                ImGui::TextDisabled("(no transform component)");
            }

            ImGui::Separator();

            // Display other components
            if (auto meshComponent = selected->GetComponent<MeshComponent>())
            {
                if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (meshComponent->GetMesh())
                    {
                        ImGui::Text("Name: %s", meshComponent->GetMesh()->GetName().c_str());
                    }
                    else
                    {
                        ImGui::TextDisabled("No Mesh Assigned");
                    }
                }
            }

            if (auto matComponent = selected->GetComponent<MaterialComponent>())
            {
                if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (matComponent->GetMaterial())
                    {
                        ImGui::Text("Name: %s", matComponent->GetMaterial()->GetName().c_str());
                    }
                    else
                    {
                        ImGui::TextDisabled("No Material Assigned");
                    }
                }
            }
        }
        else
        {
            ImGui::TextDisabled("No selection");
        }
    }

    ImGui::End();
}

void InspectorPanel::CleanUp()
{
    
}


 
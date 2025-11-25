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

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove   |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking;

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
            if (auto tc = selected->GetComponent<TransformComponent>())
            {
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    const auto& p = tc->GetPosition();
                    const auto& r = tc->GetRotation();
                    const auto& s = tc->GetScale();

                    ImGui::Text("Position: %.2f, %.2f, %.2f", p.x, p.y, p.z);
                    ImGui::Text("Rotation: %.2f, %.2f, %.2f", r.x, r.y, r.z);
                    ImGui::Text("Scale:    %.2f, %.2f, %.2f", s.x, s.y, s.z);
                }
            }
            else
            {
                ImGui::TextDisabled("(no transform component)");
            }

            ImGui::Separator();

            // Display other components
            if (auto mc = selected->GetComponent<MeshComponent>())
            {
                if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (mc->GetMesh())
                    {
                        ImGui::Text("Name: %s", mc->GetMesh()->GetName().c_str());
                    }
                    else
                    {
                        ImGui::TextDisabled("No Mesh Assigned");
                    }
                }
            }

            if (auto matc = selected->GetComponent<MaterialComponent>())
            {
                if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (matc->GetMaterial())
                    {
                        ImGui::Text("Name: %s", matc->GetMaterial()->GetName().c_str());
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


 
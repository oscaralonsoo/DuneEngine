#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "TransformComponent.h"
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
        GameObject* selected = scene->GetSelected();
        if (selected)
        {
            ImGui::Text("Name: %s", selected->GetName().c_str());
            ImGui::Separator();

            // Read transform from TransformComponent
            if (auto tc = selected->GetComponent<TransformComponent>())
            {
                const auto& p = tc->GetPosition();
                const auto& r = tc->GetRotation();
                const auto& s = tc->GetScale();

                ImGui::Text("Position: %.2f, %.2f, %.2f", p.x, p.y, p.z);
                ImGui::Text("Rotation: %.2f, %.2f, %.2f", r.x, r.y, r.z);
                ImGui::Text("Scale:    %.2f, %.2f, %.2f", s.x, s.y, s.z);
            }
            else
            {
                ImGui::TextDisabled("(no transform component)");
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

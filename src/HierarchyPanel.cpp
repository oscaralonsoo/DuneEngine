#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include <imgui.h>
#include <algorithm>

bool HierarchyPanel::Start()
{
    return true;
}

void HierarchyPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredWidth = workSize.x * HierarchyPanel::kDefaultFraction;
    float otherPanelEstimate = workSize.x * HierarchyPanel::kDefaultFraction;
    float maxAllowed = workSize.x - otherPanelEstimate - HierarchyPanel::kMinCenterWidth;
    float panelWidth = std::clamp(desiredWidth, HierarchyPanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(workPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Flags: panel fijo, sin docking ni resize
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove   |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking;

                             
    ImGui::Begin("Hierarchy", nullptr, flags);

    if (auto scene = Engine::GetInstance().scene)
    {
        for (GameObject* go : scene->GetGameObjects())
        {
            ImGui::Text("%s", go->GetName().c_str());
        }
    }

    ImGui::End();
}

void HierarchyPanel::CleanUp()
{
}

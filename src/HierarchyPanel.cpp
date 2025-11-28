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

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;

                             
    ImGui::Begin("Hierarchy", nullptr, flags);

    if (auto scene = Engine::GetInstance().scene)
    {
        std::shared_ptr<GameObject> selected = scene->GetSelected();
        for (std::shared_ptr<GameObject> go : scene->GetGameObjects())
        {
            bool isSelected = (go == selected);
            if (ImGui::Selectable(go->GetName().c_str(), isSelected))
            {
                scene->SetSelected(go);
            }
        }
    }

    ImGui::End();
}

void HierarchyPanel::CleanUp()
{
}

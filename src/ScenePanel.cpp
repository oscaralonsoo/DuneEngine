#include "ScenePanel.h"
#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleScene.h"
#include <imgui.h>
#include <algorithm>
#include "RendererAPI.h"
#include "MaterialComponent.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include "Renderer.h"

bool ScenePanel::Start()
{
    return true;
}

void ScenePanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float desiredWidth = workSize.x * ScenePanel::kDefaultFraction;
    float otherPanelEstimate = workSize.x * ScenePanel::kDefaultFraction;
    float maxAllowed = workSize.x - otherPanelEstimate - ScenePanel::kMinCenterWidth;
    float panelWidth = std::clamp(desiredWidth, ScenePanel::kMinPanelWidth, maxAllowed);

    ImGui::SetNextWindowPos(workPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysUseWindowPadding |
                            ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Scene", nullptr, flags);
    ImGui::End();
}
void ScenePanel::CleanUp()
{
}
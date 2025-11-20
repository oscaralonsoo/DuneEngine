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

    constexpr float PanelFraction = 0.18f;   // fracción del ancho de la ventana
    constexpr float MinCenterWidth = 180.0f; // ancho mínimo reservado para la zona central
    constexpr float MinPanelWidth = 80.0f;   // ancho mínimo del panel

    // Calcular y limitar el ancho del panel para evitar solapamientos
    float desiredWidth = workSize.x * PanelFraction;
    float otherPanelEstimate = workSize.x * PanelFraction; // estimación simétrica
    float maxAllowed = workSize.x - otherPanelEstimate - MinCenterWidth;
    float panelWidth = std::clamp(desiredWidth, MinPanelWidth, maxAllowed);

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

#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
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

    float panelWidth  = workSize.x * 0.18f; 
    float panelHeight = workSize.y;         

    float minCenterWidth = 180.0f; 
    float leftPanelWidth = workSize.x * 0.18f; 
    float maxAllowed = workSize.x - leftPanelWidth - minCenterWidth;
    if (panelWidth > maxAllowed)
    {
        panelWidth = std::max(80.0f, maxAllowed);
    }

    // Posicionar la ventana en la parte derecha
    ImVec2 pos = ImVec2(workPos.x + workSize.x - panelWidth, workPos.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking;


    ImGui::Begin("Inspector", nullptr, flags);
    auto& scene = Engine::GetInstance().scene;

    ImGui::End();
}

void InspectorPanel::CleanUp()
{
    
}

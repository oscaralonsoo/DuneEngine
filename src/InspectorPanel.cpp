#include "InspectorPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include <imgui.h>

bool InspectorPanel::Start()
{
    return true;
}

void InspectorPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos  = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    // Porcentaje de ancho que ocupa la jerarquía (como Unity)
    float panelWidth  = workSize.x * 0.18f;  // 25% de la pantalla
    float panelHeight = workSize.y;          // toda la altura

    // Posición: esquina superior izquierda del área de trabajo
    ImVec2 pos = ImVec2(workPos.x + workSize.x - panelWidth, workPos.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking;
        // NO pasamos bandera de NoTitleBar → se ve el título

    // nullptr → título SÍ, pero SIN cruz de cierre
    ImGui::Begin("Hierarchy", nullptr, flags);
    auto& scene = Engine::GetInstance().scene;

    if (scene)
    {
        for (GameObject* go : scene->GetGameObjects())
        {
            ImGui::Text("%s", go->GetName().c_str());
        }
    }

    ImGui::End();
}

void InspectorPanel::CleanUp()
{
    
}

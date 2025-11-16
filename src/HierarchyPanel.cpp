#include "HierarchyPanel.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include <imgui.h>

void HierarchyPanel::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = viewport->WorkPos;
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking;

    ImGui::Begin("Hierarchy", nullptr, flags);

    for (GameObject* go : Engine::GetInstance().scene.get()->GetGameObjects())
    {
        ImGui::Text("%s", go->GetName().c_str());
    }

    ImGui::End();
}

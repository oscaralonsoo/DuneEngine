#include "PanelStats.h"
#include "Engine.h"
#include "ModuleWindow.h"
#include <imgui.h>

void PanelStats::OnImGuiRender()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = viewport->WorkPos;
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking;

    ImGui::Begin("Stats", nullptr, flags);

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f", fps);

    int width = 0;
    int height = 0;
    Engine::GetInstance().window->SetWindowSize(width, height);
    ImGui::Text("Window: %d x %d", width, height);

    ImGui::End();
}

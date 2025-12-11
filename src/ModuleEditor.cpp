#include "Engine.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include "ResourceUtils.h"
#include "Model.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include "GameTime.h" 
#include "ToolbarPanel.h"
#include "ScenePanel.h"

ModuleEditor::ModuleEditor()
{
    name = "editor";
}

bool ModuleEditor::Start()
{
    AddPanel<HierarchyPanel>();
    AddPanel<InspectorPanel>();
    AddPanel<ProjectPanel>();
    AddPanel<ToolbarPanel>();

    for (auto& panel : panels)
        panel->Start();
    
    return true;
}

bool ModuleEditor::Update()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    // Ventana invisible a pantalla completa que aloja el DockSpace
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoBackground; // deja ver tu escena
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DockSpaceHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    // Crea el DockSpace
    ImGuiID dockspace_id = ImGui::GetID("DuneEngineDockSpace");
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, ImVec2(0,0), dock_flags);
    ImGui::End();

    for (auto& panel : panels)
    {
        if (Panel* p = dynamic_cast<Panel*>(panel.get()))
            p->Render();
        else
            panel->OnImGuiRender();
    }

    return true;
}

bool ModuleEditor::CleanUp()
{
    for (auto& panel : panels)
    {
        panel->CleanUp();
    }
    panels.clear();
    return true;
}

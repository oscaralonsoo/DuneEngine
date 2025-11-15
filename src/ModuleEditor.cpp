#include "ModuleEditor.h"
#include "PanelStats.h"
#include <imgui.h>

ModuleEditor::ModuleEditor()
{
    name = "editor";
}

bool ModuleEditor::Start()
{
    AddPanel<PanelStats>();
    return true;
}

void ModuleEditor::DrawDockspace()
{
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

bool ModuleEditor::Update()
{
    DrawDockspace();

    for (auto& panel : panels)
        panel->OnImGuiRender();

    return true;
}

bool ModuleEditor::CleanUp()
{
    panels.clear();
    return true;
}

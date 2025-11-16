#include "ModuleEditor.h"
#include "PanelStats.h"
#include "HierarchyPanel.h"
#include <imgui.h>

ModuleEditor::ModuleEditor()
{
    name = "editor";
}

bool ModuleEditor::Start()
{
    AddPanel<HierarchyPanel>();
    return true;
}

bool ModuleEditor::Update()
{

    for (auto& panel : panels)
        panel->OnImGuiRender();

    return true;
}

bool ModuleEditor::CleanUp()
{
    panels.clear();
    return true;
}

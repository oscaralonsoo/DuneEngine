#include "ModuleEditor.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include <imgui.h>

ModuleEditor::ModuleEditor()
{
    name = "editor";
}

bool ModuleEditor::Start()
{
    AddPanel<HierarchyPanel>();
    AddPanel<InspectorPanel>();

    for (auto& panel : panels)
        panel->Start();
    
    return true;
}

bool ModuleEditor::Update()
{

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

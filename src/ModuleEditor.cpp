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

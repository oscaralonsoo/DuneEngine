#pragma once

#include "ModuleEditor.h"

class HierarchyPanel : public Panel
{
public:
    HierarchyPanel()
        : Panel("Hierarchy")
    {}

    bool Start() override;
    void OnImGuiRender() override;
    void CleanUp() override;
};

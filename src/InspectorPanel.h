#pragma once

#include "ModuleEditor.h"

class InspectorPanel : public Panel
{
    public:
        InspectorPanel()
            : Panel("Inspector")
        {}

        bool Start() override;
        void OnImGuiRender() override;
        void CleanUp() override;
};

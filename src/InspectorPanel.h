#pragma once

#include "ModuleEditor.h"

class InspectorPanel : public EditorPanel
{
    public:
        InspectorPanel()
        {}

        bool Start() override;
        void OnImGuiRender() override;
        void CleanUp() override;
};

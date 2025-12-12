#pragma once

#include "ModuleEditor.h"
#include "Component.h"

class InspectorPanel : public EditorPanel
{
    public:
        InspectorPanel()
        {}

        bool Start() override;
        void OnImGuiRender() override;
        void CleanUp() override;

    private:
        void RenderComponent(Component* component, float panelWidth);
};

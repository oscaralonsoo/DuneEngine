#pragma once

#include "ModuleEditor.h"

class InspectorPanel : public EditorPanel
{
    public:
        InspectorPanel()
        {}

        //initial sizes and limits
        static constexpr float kDefaultFraction = 0.18f; // fracción del ancho de la ventana
        static constexpr float kMinPanelWidth = 80.0f;   // ancho mínimo del panel en px
        static constexpr float kMinCenterWidth = 180.0f; // ancho mínimo reservado para zona central

        bool Start() override;
        void OnImGuiRender() override;
        void CleanUp() override;
};

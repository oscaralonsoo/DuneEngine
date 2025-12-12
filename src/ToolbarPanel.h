#pragma once

#include "ModuleEditor.h"   // Aquí está definida EditorPanel
#include <imgui.h>
#include "GameTime.h"

// Panel que dibuja la barra de herramientas (Play/Pause/Stop/Step/TimeScale)
class ToolbarPanel : public EditorPanel
{
public:
    ToolbarPanel() = default;
    ~ToolbarPanel() = default;

    bool Start() override { return true; }

    void OnImGuiRender() override;
};

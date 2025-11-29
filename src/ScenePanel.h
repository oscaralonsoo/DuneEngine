#pragma once

#include "ModuleEditor.h"
#include "imgui.h"
#include <memory>
#include <algorithm>

class ScenePanel : public EditorPanel
{
public:
    ScenePanel() {}

    // initial sizes and limits
    static constexpr float kDefaultFraction = 0.18f;
    static constexpr float kMinPanelWidth = 80.0f;
    static constexpr float kMinCenterWidth = 180.0f;

    bool Start() override;
    void OnImGuiRender() override;
    void CleanUp() override;

private:
};

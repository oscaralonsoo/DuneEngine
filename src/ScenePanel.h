#pragma once

#include "ModuleEditor.h"
#include <imgui.h>
#include "GameTime.h"
#include <memory>
#include "Framebuffer.h"

// ScenePanel that functions like Unity's Scene and Game panels
class ScenePanel : public EditorPanel
{
public:
    ScenePanel() = default;
    ~ScenePanel() = default;

    bool Start() override;
    void OnImGuiRender() override;
    void CleanUp() override;

private:
    void RenderSceneView();
    void RenderGameView();
    void RenderToolbarControls();

    bool m_ShowSceneView = true;
    bool m_ShowGameView = false;
    Framebuffer* m_SceneFramebuffer = nullptr;
    Framebuffer* m_GameFramebuffer  = nullptr;


    // Panel sizing constants
    static constexpr float kDefaultFraction = 0.18f;
    static constexpr float kMinPanelWidth = 80.0f;
    static constexpr float kMinCenterWidth = 180.0f;
};

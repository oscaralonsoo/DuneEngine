#pragma once

#include "ModuleEditor.h"
#include <imgui.h>
#include "GameTime.h"
#include <memory>
#include "Framebuffer.h"
#include <filesystem>

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
    void HandleSceneDragDrop(ImVec2 sceneViewSize, float mouseX, float mouseY);
    void HandleAssetDrop(const std::filesystem::path& assetPath, float mouseX = -1.0f, float mouseY = -1.0f);

    bool m_ShowSceneView = true;
    bool m_ShowGameView = false;
    float m_SceneViewHeight = 400.0f;
    float m_GameViewHeight = 400.0f;

    // Panel sizing constants
    static constexpr float kDefaultFraction = 0.18f;
    static constexpr float kMinPanelWidth = 80.0f;
    static constexpr float kMinCenterWidth = 180.0f;
};

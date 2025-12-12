#pragma once

#include "ModuleEditor.h"
#include <imgui.h>
#include "GameTime.h"
#include <memory>
#include "Framebuffer.h"
#include <filesystem>

enum class ViewType { Scene, Game };

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

    Framebuffer* m_SceneFramebuffer = nullptr;
    Framebuffer* m_GameFramebuffer  = nullptr;
    bool m_ForceRestoreView = false;
    ViewType m_CurrentView = ViewType::Scene;
    ViewType m_LastViewBeforePlay = ViewType::Scene;
};

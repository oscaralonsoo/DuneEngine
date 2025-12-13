#pragma once

#include "ModuleEditor.h"
#include <memory>

// Forward declarations
class SceneViewController;
class SceneInteractionHandler;
class SceneAssetHandler;
class ScenePanelUI;

class ScenePanel : public EditorPanel
{
public:
    ScenePanel();
    ~ScenePanel();

    bool Start() override;
    void OnImGuiRender() override;
    void CleanUp() override;

private:
    void RenderSceneView();
    void RenderGameView();
    void RenderToolbarControls();

    std::unique_ptr<SceneViewController> m_ViewController;
    std::unique_ptr<SceneInteractionHandler> m_InteractionHandler;
    std::unique_ptr<SceneAssetHandler> m_AssetHandler;
    std::unique_ptr<ScenePanelUI> m_UI;
};

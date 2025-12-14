#pragma once

#include "ModuleEditor.h"
#include <memory>

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

    void OnImGuiRender() override;     // Render the panel UI

    void CleanUp() override;

private:

    // Renders the Scene (editor) view
    void RenderSceneView();

    // Renders the Game (play mode) view
    void RenderGameView();

    // Renders the top toolbar (Play / Pause / Step)
    void RenderToolbarControls();

    // Controllers and UI for the panel
    std::unique_ptr<SceneViewController> m_ViewController;
    std::unique_ptr<SceneInteractionHandler> m_InteractionHandler;
    std::unique_ptr<SceneAssetHandler> m_AssetHandler;
    std::unique_ptr<ScenePanelUI> m_UI;
};

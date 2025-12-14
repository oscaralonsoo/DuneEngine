#pragma once

#include <cstdint>

class SceneInteractionHandler
{
public:
    SceneInteractionHandler() = default;
    ~SceneInteractionHandler() = default;

    void HandleMouseClick(float mouseX, float mouseY, uint32_t width, uint32_t height);
    void HandleMouseRelease();
    void HandleKeyboardShortcuts();
    void UpdateGizmo(float mouseX, float mouseY);

    bool IsSceneEditable() const;

private:
    bool HandleGizmoInteraction(float mouseX, float mouseY);
    void PerformObjectPicking(float mouseX, float mouseY, uint32_t width, uint32_t height);
};

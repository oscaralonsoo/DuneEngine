#pragma once

#include <cstdint>

class SceneInteractionHandler
{
public:
    SceneInteractionHandler() = default;
    ~SceneInteractionHandler() = default;

    // Handles mouse click events
    void HandleMouseClick(float mouseX, float mouseY, uint32_t width, uint32_t height);

    // Handles mouse release events
    void HandleMouseRelease();

    // Handles keyboard shortcuts for gizmo modes
    void HandleKeyboardShortcuts();

    // Updates the gizmo with the current mouse position
    void UpdateGizmo(float mouseX, float mouseY);

    // Returns true if the Scene is editable
    bool IsSceneEditable() const;

private:
    // Handles gizmo interaction, returns true if gizmo consumed the click
    bool HandleGizmoInteraction(float mouseX, float mouseY);

    // Picks an object in the scene under the mouse cursor
    void PerformObjectPicking(float mouseX, float mouseY, uint32_t width, uint32_t height);
};

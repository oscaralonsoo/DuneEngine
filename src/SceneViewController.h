#pragma once

#include "Framebuffer.h"
#include <cstdint>

enum class ViewType { Scene, Game };

class SceneViewController
{
public:
    SceneViewController() = default;
    ~SceneViewController() = default;

    bool Initialize(uint32_t width, uint32_t height);

    void Cleanup();

    // Returns the current active view
    ViewType GetCurrentView() const { return m_CurrentView; }

    // Sets the active view
    void SetCurrentView(ViewType view) { m_CurrentView = view; }

    // Returns the Scene framebuffer
    Framebuffer* GetSceneFramebuffer() { return m_SceneFramebuffer; }

    // Returns the Game framebuffer
    Framebuffer* GetGameFramebuffer() { return m_GameFramebuffer; }

    // Resizes a framebuffer if its size has changed
    void ResizeFramebufferIfNeeded(Framebuffer* framebuffer, uint32_t width, uint32_t height);

    // Switches to Game view when play mode starts
    void OnPlayModeStarted();

    // Restores previous view when play mode stops
    void OnPlayModeStopped();

    // Returns true if the view must be restored
    bool ShouldForceRestoreView() const { return m_ForceRestoreView; }

    // Clears the restore view flag
    void ClearForceRestoreView() { m_ForceRestoreView = false; }

private:
private:
    // Framebuffers for Scene and Game views
    Framebuffer* m_SceneFramebuffer = nullptr;
    Framebuffer* m_GameFramebuffer = nullptr;

    // Current and previous active views
    ViewType m_CurrentView = ViewType::Scene;
    ViewType m_LastViewBeforePlay = ViewType::Scene;

    // Forces UI to restore previous view
    bool m_ForceRestoreView = false;

};

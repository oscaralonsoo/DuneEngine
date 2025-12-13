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

    ViewType GetCurrentView() const { return m_CurrentView; }
    void SetCurrentView(ViewType view) { m_CurrentView = view; }

    Framebuffer* GetSceneFramebuffer() { return m_SceneFramebuffer; }
    Framebuffer* GetGameFramebuffer() { return m_GameFramebuffer; }

    void ResizeFramebufferIfNeeded(Framebuffer* framebuffer, uint32_t width, uint32_t height);

    void OnPlayModeStarted();
    void OnPlayModeStopped();

    bool ShouldForceRestoreView() const { return m_ForceRestoreView; }
    void ClearForceRestoreView() { m_ForceRestoreView = false; }

private:
    Framebuffer* m_SceneFramebuffer = nullptr;
    Framebuffer* m_GameFramebuffer = nullptr;
    ViewType m_CurrentView = ViewType::Scene;
    ViewType m_LastViewBeforePlay = ViewType::Scene;
    bool m_ForceRestoreView = false;
};

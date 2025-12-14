#include "SceneViewController.h"

bool SceneViewController::Initialize(uint32_t width, uint32_t height)
{
    m_SceneFramebuffer = new Framebuffer(width, height);
    m_GameFramebuffer = new Framebuffer(width, height);
    return true;
}

void SceneViewController::Cleanup()
{
    if (m_SceneFramebuffer)
    {
        delete m_SceneFramebuffer;
        m_SceneFramebuffer = nullptr;
    }
    if (m_GameFramebuffer)
    {
        delete m_GameFramebuffer;
        m_GameFramebuffer = nullptr;
    }
}

void SceneViewController::ResizeFramebufferIfNeeded(Framebuffer* framebuffer, uint32_t width, uint32_t height)
{
    if (framebuffer && width > 0 && height > 0 &&
        (framebuffer->GetWidth() != width || framebuffer->GetHeight() != height))
    {
        framebuffer->Resize(width, height);
    }
}

void SceneViewController::OnPlayModeStarted()
{
    m_LastViewBeforePlay = m_CurrentView;
    m_CurrentView = ViewType::Game;
}

void SceneViewController::OnPlayModeStopped()
{
    m_CurrentView = m_LastViewBeforePlay;
    m_ForceRestoreView = true;
}

#pragma once

#include <imgui.h>
#include <functional>
#include "Framebuffer.h"
#include "CameraComponent.h"

enum class ViewType;

class ScenePanelUI
{
public:
    ScenePanelUI() = default;
    ~ScenePanelUI() = default;

    void RenderToolbar(
        std::function<void()> onPlayStop,
        std::function<void()> onPause,
        std::function<void()> onStep
    );

    void RenderSceneView(
        Framebuffer* framebuffer,
        std::function<void(float, float, bool)> onMouseInteraction,
        std::function<void()> onKeyboardInput,
        std::function<void(ImVec2, float, float)> onDragDrop
    );

    void RenderGameView(
        Framebuffer* framebuffer,
        CameraComponent* camera,
        class GameObject* cameraGameObject
    );

    void RenderTabBar(
        ViewType currentView,
        bool isPlaying,
        bool forceRestore,
        std::function<void(ViewType)> onViewChanged
    );

private:
    bool m_IsDragging = false;
};

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

    // Top toolbar: Play / Pause / Step
    void RenderToolbar(
        std::function<void()> onPlayStop,
        std::function<void()> onPause,
        std::function<void()> onStep
    );

    // Scene view with mouse, keyboard and drag & drop interactions
    void RenderSceneView(
        Framebuffer* framebuffer,
        std::function<void(float, float, bool)> onMouseInteraction,
        std::function<void()> onKeyboardInput,
        std::function<void(ImVec2, float, float)> onDragDrop
    );

    // Game view rendering using active camera
    void RenderGameView(
        Framebuffer* framebuffer,
        CameraComponent* camera,
        class GameObject* cameraGameObject
    );

    // Tabs to switch between Scene and Game views
    void RenderTabBar(
        ViewType currentView,
        bool isPlaying,
        bool forceRestore,
        std::function<void(ViewType)> onViewChanged
    );

private:
    // True while dragging inside the Scene view
    bool m_IsDragging = false;
};

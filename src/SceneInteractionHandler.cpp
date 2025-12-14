#include "SceneInteractionHandler.h"
#include "Engine.h"
#include "ModuleScene.h"
#include "Gizmo.h"
#include "GameTime.h"
#include <imgui.h>

void SceneInteractionHandler::HandleMouseClick(float mouseX, float mouseY, uint32_t width, uint32_t height)
{
    if (!IsSceneEditable())
        return;

    bool consumedByGizmo = HandleGizmoInteraction(mouseX, mouseY);

    if (!consumedByGizmo)
    {
        PerformObjectPicking(mouseX, mouseY, width, height);
    }
}

void SceneInteractionHandler::HandleMouseRelease()
{
    auto& engine = Engine::GetInstance();
    if (engine.gizmo)
    {
        engine.gizmo->OnMouseUp();
    }
}

void SceneInteractionHandler::HandleKeyboardShortcuts()
{
    if (!IsSceneEditable())
        return;

    auto& engine = Engine::GetInstance();
    if (!engine.gizmo)
        return;

    // Si el gizmo está siendo usado (hovering o dragging), cambiar modo según el eje/parte tocada
    if (engine.gizmo->IsHovered() || engine.gizmo->IsDragging())
    {
        GizmoMode currentMode = engine.gizmo->GetMode();
        
        // W: Si estamos en otro modo y tocando el gizmo, cambiar a Translate
        if (ImGui::IsKeyPressed(ImGuiKey_W) && currentMode != GizmoMode::Translate)
        {
            engine.gizmo->SetMode(GizmoMode::Translate);
        }
        // E: Si estamos en otro modo y tocando el gizmo, cambiar a Rotate
        else if (ImGui::IsKeyPressed(ImGuiKey_E) && currentMode != GizmoMode::Rotate)
        {
            engine.gizmo->SetMode(GizmoMode::Rotate);
        }
        // R: Si estamos en otro modo y tocando el gizmo, cambiar a Scale
        else if (ImGui::IsKeyPressed(ImGuiKey_R) && currentMode != GizmoMode::Scale)
        {
            engine.gizmo->SetMode(GizmoMode::Scale);
        }
        // Q: Desactivar gizmo
        else if (ImGui::IsKeyPressed(ImGuiKey_Q))
        {
            engine.gizmo->SetMode(GizmoMode::None);
        }
    }
    else
    {
        // Comportamiento normal cuando NO estamos tocando el gizmo
        if (ImGui::IsKeyPressed(ImGuiKey_W))
            engine.gizmo->SetMode(GizmoMode::Translate);
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            engine.gizmo->SetMode(GizmoMode::Rotate);
        if (ImGui::IsKeyPressed(ImGuiKey_R))
            engine.gizmo->SetMode(GizmoMode::Scale);
        if (ImGui::IsKeyPressed(ImGuiKey_Q))
            engine.gizmo->SetMode(GizmoMode::None);
    }
}

void SceneInteractionHandler::UpdateGizmo(float mouseX, float mouseY)
{
    auto& engine = Engine::GetInstance();
    if (engine.gizmo)
    {
        engine.gizmo->Update(mouseX, mouseY);
    }
}

bool SceneInteractionHandler::IsSceneEditable() const
{
    return !GameTime::IsPlaying();
}

bool SceneInteractionHandler::HandleGizmoInteraction(float mouseX, float mouseY)
{
    auto& engine = Engine::GetInstance();
    if (engine.gizmo)
    {
        return engine.gizmo->OnMouseDown(mouseX, mouseY);
    }
    return false;
}

void SceneInteractionHandler::PerformObjectPicking(float mouseX, float mouseY, uint32_t width, uint32_t height)
{
    auto& engine = Engine::GetInstance();
    auto* scene = engine.scene.get();
    if (!scene)
        return;

    auto picked = scene->GetRaycaster()->PickObject(
        mouseX, mouseY,
        width, height,
        scene->GetGameObjects()
    );

    scene->SetSelected(picked);
}

#pragma once
#include <glm/glm.hpp>
#include <memory>

class GameObject;
class TransformComponent;

enum class GizmoMode { Translate, Rotate, Scale };
enum class GizmoAxis { None, X, Y, Z };

class Gizmo
{
public:
    void SetMode(GizmoMode mode) { mMode = mode; }
    GizmoMode GetMode() const { return mMode; }

    // Llamar cada frame (para hotkeys W/E/R + hover)
    void Update(float mouseX, float mouseY);

    // Devuelve true si el gizmo “consume” el click (empieza drag)
    bool OnMouseDown(float mouseX, float mouseY);

    void OnMouseUp();

    // Render 3D overlay
    void Render();

    bool IsDragging() const { return mDragging; }
    bool IsHovered()  const { return mHoveredAxis != GizmoAxis::None; }

private:
    // Helpers
    std::shared_ptr<GameObject> GetSelected() const;
    TransformComponent* GetSelectedTransform() const;

    glm::vec3 GetGizmoOriginWS() const;
    float     GetGizmoScaleWS(const glm::vec3& origin) const;

    // Picking y dragging
    GizmoAxis PickAxis(float mouseX, float mouseY, const glm::vec3& origin, float scale) const;
    bool RayPlaneIntersection(const glm::vec3& rayO, const glm::vec3& rayD,
                              const glm::vec3& planeP, const glm::vec3& planeN,
                              glm::vec3& outHit) const;

    glm::vec3 GetCameraForwardWS() const;
    glm::vec3 GetCameraPosWS() const;

    glm::vec3 AxisDirWS(GizmoAxis axis) const;

private:
    GizmoMode mMode = GizmoMode::Translate;

    GizmoAxis mHoveredAxis = GizmoAxis::None;
    GizmoAxis mActiveAxis  = GizmoAxis::None;

    bool mDragging = false;

    // Drag state
    glm::vec3 mDragStartHitWS{0.0f};
    glm::vec3 mDragStartPosWS{0.0f};
};
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class GameObject;
class TransformComponent;

enum class GizmoMode { None, Translate, Rotate, Scale };
enum class GizmoAxis { None, X, Y, Z, Center, PlaneXY, PlaneXZ, PlaneYZ };

class Gizmo
{
public:
    void SetMode(GizmoMode mode) { mMode = mode; }
    GizmoMode GetMode() const { return mMode; }

    void Update(float mouseX, float mouseY);
    bool OnMouseDown(float mouseX, float mouseY);
    void OnMouseUp();
    void Render();

    bool IsDragging() const { return mDragging; }
    bool IsHovered()  const { return mHoveredAxis != GizmoAxis::None; }

    void SetTranslateSnap(float s) { mTranslateSnap = s; }
    float GetTranslateSnap() const { return mTranslateSnap; }

private:
    std::shared_ptr<GameObject> GetSelected() const;
    TransformComponent* GetSelectedTransform() const;

    glm::vec3 GetGizmoOriginWS() const;
    float     GetGizmoScaleWS(const glm::vec3& origin) const;

    GizmoAxis PickAxis(float mouseX, float mouseY, const glm::vec3& origin, float scale) const;
    GizmoAxis PickRotateAxis(float mouseX, float mouseY, const glm::vec3& origin, float scale) const;

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

    // Drag common
    glm::vec3 mDragStartHitWS{0.0f};

    // Translate snap
    float mTranslateSnap = 0.5f;

    // Translate plane normal (para XY/XZ/YZ)
    glm::vec3 mTranslatePlaneN{0.0f, 0.0f, 1.0f};

    // Rotate state
    glm::vec3 mRotatePlaneN{0.0f};
    glm::vec3 mRotateStartVec{0.0f};
    glm::quat mRotateStartWorld{1,0,0,0};

    // Scale state
    glm::vec3 mScaleStartLocal{1.0f, 1.0f, 1.0f};
    float     mUniformScaleStartDist = 1.0f;
};
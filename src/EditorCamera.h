#pragma once

#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class EditorCamera : public Camera, public ICamera
{
public:
    enum class CameraState
    {
        NONE,
        ORBIT,
        FLY
    };

    EditorCamera() = default;
    EditorCamera(float fov, float aspectRatio = 1.778f, float nearClip = 0.1f, float farClip = 1000.0f);

    void Update(/*float dt*/);

    inline void SetFocusPoint(glm::vec3 focusPoint) { mFocalPoint = focusPoint; }

    // ICamera
    const glm::mat4& GetViewMatrix() const override { return mViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const override { return Camera::GetProjectionMatrix(); }
    glm::vec3 GetPosition() const override { return mPosition; }
    void SetViewportSize(float width, float height) override { Camera::SetViewportSize(width, height); }

    // Utilidades extra específicas del editor
    glm::vec3 GetUpDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetForwardDirection() const;

    CameraState GetState() const { return mCurrentState; }
    glm::quat GetOrientation() const;

    float GetFlySpeed() const { return mCurrentSpeed; }
    float GetOrbitZoom() const { return mDistance; }

private:
    void UpdateView();
    glm::vec3 CalculatePosition() const;

    void MouseRotate(const glm::vec2& delta);
    void MousePan(const glm::vec2 &delta);
    void MouseZoom(float delta);
    void Fly(const glm::vec2 &mouseDelta);

private:
    glm::mat4 mViewMatrix{1.0f};

    glm::vec3 mPosition   {0.0f, 0.0f, 0.0f};
    glm::vec3 mFocalPoint {0.0f, 0.0f, 0.0f};

    float mDistance     = 10.0f;
    float mBaseSpeed    = 0.02f;
    float mCurrentSpeed = mBaseSpeed;
    float mPitch        = 0.0f;
    float mYaw          = 0.0f;

    glm::vec2 mInitialMousePosition{0.0f, 0.0f};

    CameraState mCurrentState = CameraState::NONE;
};

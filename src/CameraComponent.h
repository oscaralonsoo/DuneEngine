#pragma once

#include "Component.h"
#include "Camera.h"
#include <glm/glm.hpp>

class CameraComponent : public Component, public ICamera
{
public:
    CameraComponent(GameObject* owner);
    ~CameraComponent() = default;

    bool Update() override;
    void OnInspectorRender(float panelWidth) override;

    // Acceso a parámetros de proyección
    float GetFOV() const             { return mCamera.GetFOV(); }
    void  SetFOV(float fov)          { mCamera.SetFOV(fov); }

    float GetNearClip() const        { return mCamera.GetNearClip(); }
    void  SetNearClip(float value)   { mCamera.SetNearClip(value); }

    float GetFarClip() const         { return mCamera.GetFarClip(); }
    void  SetFarClip(float value)    { mCamera.SetFarClip(value); }

    // ICamera
    const glm::mat4& GetViewMatrix() const override      { return mViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const override{ return mCamera.GetProjectionMatrix(); }
    glm::vec3 GetPosition() const override               { return mPosition; }
    void SetViewportSize(float width, float height) override
    {
        mCamera.SetViewportSize(width, height);
    }

private:
    void UpdateFromTransform();

private:
    Camera     mCamera;
    glm::mat4  mViewMatrix{1.0f};
    glm::vec3  mPosition{0.0f, 0.0f, 0.0f};
};
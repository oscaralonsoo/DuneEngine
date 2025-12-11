#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Interfaz común de cámara para el renderer
class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual const glm::mat4& GetViewMatrix() const = 0;
    virtual const glm::mat4& GetProjectionMatrix() const = 0;
    virtual glm::vec3 GetPosition() const = 0;

    virtual void SetViewportSize(float width, float height) = 0;
};

// Cámara “base” con solo proyección
class Camera
{
public:
    Camera();
    Camera(const glm::mat4 &projection)
        : mProjection(projection) {}
    virtual ~Camera() = default;

    float GetFOV() const;
    void SetFOV(float fov);

    float GetAspectRatio() const;
    void SetAspectRatio(float aspectRatio);

    float GetNearClip() const;
    void SetNearClip(float nearClip);

    float GetFarClip() const;
    void SetFarClip(float farClip);

    const glm::mat4 &GetProjectionMatrix() const;

    glm::vec2 GetViewportSize() const;
    void SetViewportSize(float width, float height);

private:
    void UpdateProjection();

protected:
    float mFOV = 45.0f;
    float mAspectRatio = 1.778f;
    float mNearClip = 0.1f;
    float mFarClip = 1000.0f;

    glm::mat4 mProjection = glm::mat4(1.0f);

    float mViewportWidth = 1280.0f;
    float mViewportHeight = 720.0f;
};
#include "Camera.h"

Camera::Camera()
{
    UpdateProjection();
}

float Camera::GetFOV() const
{
    return mFOV;
}

void Camera::SetFOV(float fov)
{
    mFOV = fov;
    UpdateProjection();
}

float Camera::GetAspectRatio() const
{
    return mAspectRatio;
}

void Camera::SetAspectRatio(float aspectRatio)
{
    mAspectRatio = aspectRatio;
    UpdateProjection();
}

float Camera::GetNearClip() const
{
    return mNearClip;
}

void Camera::SetNearClip(float nearClip)
{
    mNearClip = nearClip;
    UpdateProjection();
}

float Camera::GetFarClip() const
{
    return mFarClip;
}

void Camera::SetFarClip(float farClip)
{
    mFarClip = farClip;
    UpdateProjection();
}

const glm::mat4 &Camera::GetProjectionMatrix() const
{
    return mProjection;
}

glm::vec2 Camera::GetViewportSize() const
{
    return {mViewportWidth, mViewportHeight};
}

void Camera::SetViewportSize(float width, float height)
{
    mViewportWidth = width;
    mViewportHeight = height;
    UpdateProjection();
}

void Camera::UpdateProjection()
{
    mAspectRatio = mViewportWidth / mViewportHeight;
    mProjection = glm::perspective(glm::radians(mFOV), mAspectRatio, mNearClip, mFarClip);
}

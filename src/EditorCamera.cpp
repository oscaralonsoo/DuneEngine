#include "EditorCamera.h"
#include "ModuleInput.h"
#include "Engine.h"
#include <SDL3/SDL_scancode.h>

EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
{
    mFOV = fov;
    mAspectRatio = aspectRatio;
    mNearClip = nearClip;
    mFarClip = farClip;

    UpdateView();
}

void EditorCamera::Update(/*float dt*/)
{
    ModuleInput *input = Engine::GetInstance().input.get();

    SDL_Point mousePos = input->GetMousePosition();
    glm::vec2 currentMousePos((float)mousePos.x, (float)mousePos.y);
    glm::vec2 delta = (currentMousePos - mInitialMousePosition) * 0.01f;
    mInitialMousePosition = currentMousePos;

    KeyState middleButton = input->GetMouseButtonDown(2);
    KeyState rightButton = input->GetMouseButtonDown(3);

    if (middleButton == KEY_DOWN || middleButton == KEY_REPEAT)
    {
        if (input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
        {
            MousePan(delta);
        }
        else
        {
            MouseRotate(delta);
        }
    }
    else if (rightButton == KEY_DOWN || rightButton == KEY_REPEAT)
    {
        mCurrentState = CameraState::FLY;
        Fly(delta);
    }
    else
    {
        mCurrentState = CameraState::ORBIT;
    }

    UpdateView();
}

void EditorCamera::Fly(const glm::vec2 &mouseDelta)
{
    MouseRotate(mouseDelta);

    ModuleInput *input = Engine::GetInstance().input.get();

    if (input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
        mCurrentSpeed = mBaseSpeed * 2.0f;
    else
        mCurrentSpeed = mBaseSpeed;

    if (input->GetKey(SDL_SCANCODE_W) == KEY_DOWN || input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
        mPosition += GetForwardDirection() * mCurrentSpeed;

    if (input->GetKey(SDL_SCANCODE_S) == KEY_DOWN || input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
        mPosition -= GetForwardDirection() * mCurrentSpeed;

    if (input->GetKey(SDL_SCANCODE_A) == KEY_DOWN || input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
        mPosition -= GetRightDirection() * mCurrentSpeed;

    if (input->GetKey(SDL_SCANCODE_D) == KEY_DOWN || input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
        mPosition += GetRightDirection() * mCurrentSpeed;

    if (input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN || input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
        mPosition -= GetUpDirection() * mCurrentSpeed;

    if (input->GetKey(SDL_SCANCODE_E) == KEY_DOWN || input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
        mPosition += GetUpDirection() * mCurrentSpeed;

    mFocalPoint = mPosition + GetForwardDirection() * mDistance;
    UpdateView();
}

void EditorCamera::MousePan(const glm::vec2 &delta)
{
    mFocalPoint += -GetRightDirection() * delta.x * mDistance * 0.1f;
    mFocalPoint += GetUpDirection() * delta.y * mDistance * 0.1f;
}

void EditorCamera::MouseZoom(float delta)
{
    if (mCurrentState == CameraState::FLY)
    {
        mPosition += GetForwardDirection() * delta;
    }
    else if (mCurrentState == CameraState::ORBIT)
    {
        mDistance -= delta;
        if (mDistance < 0.1f)
        {
            mDistance = 0.1f;
        }
        if (mDistance > 100.0f)
        {
            mDistance = 100.0f;
        }
    }
}

void EditorCamera::MouseRotate(const glm::vec2 &delta)
{
    mYaw += delta.x;
    mPitch += delta.y;

    float pitchLimit = glm::radians(89.0f);
    mPitch = glm::clamp(mPitch, -pitchLimit, pitchLimit);
}

void EditorCamera::UpdateView()
{
    if (mCurrentState == CameraState::FLY)
    {
        mViewMatrix = glm::lookAt(mPosition, mPosition + GetForwardDirection(), GetUpDirection());
    }
    else if (mCurrentState == CameraState::ORBIT)
    {
        mPosition = CalculatePosition();
        mViewMatrix = glm::lookAt(mPosition, mFocalPoint, GetUpDirection());
    }
}

glm::vec3 EditorCamera::GetUpDirection() const
{
    return GetOrientation() * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 EditorCamera::GetRightDirection() const
{
    return GetOrientation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 EditorCamera::GetForwardDirection() const
{
    return GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::quat EditorCamera::GetOrientation() const
{
    return glm::quat(glm::vec3(-mPitch, -mYaw, 0.0f));
}

glm::vec3 EditorCamera::CalculatePosition() const
{
    return mFocalPoint - GetForwardDirection() * mDistance;
}

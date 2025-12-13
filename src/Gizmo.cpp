#include "Gizmo.h"

#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleRenderer.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "Raycaster.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include "Shader.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

static glm::vec3 SafeNormalize(const glm::vec3& v, const glm::vec3& fallback);

static void PushTri(std::vector<glm::vec3>& out, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

static void AddThickSegmentBillboard(std::vector<glm::vec3>& out,
                                     const glm::vec3& a,
                                     const glm::vec3& b,
                                     const glm::vec3& camForward,
                                     float halfThicknessWorld);

static void AddArrowHead(std::vector<glm::vec3>& out,
                         const glm::vec3& tip,
                         const glm::vec3& dir,
                         const glm::vec3& camForward,
                         float length,
                         float radius);

static void AddSquareHandle(std::vector<glm::vec3>& out,
                            const glm::vec3& center,
                            const glm::vec3& camForward,
                            const glm::vec3& axisDir,
                            float halfSize);

static void AddPlaneQuad(std::vector<glm::vec3>& out,
                         const glm::vec3& origin,
                         const glm::vec3& uDir,
                         const glm::vec3& vDir,
                         float u0, float u1,
                         float v0, float v1);

static glm::vec3 ExtractTranslation(const glm::mat4& m);
static glm::quat ExtractWorldRotation(const glm::mat4& world);

static void SetWorldPosition(TransformComponent* tc, const glm::vec3& worldPos);
static void SetLocalRotationFromWorld(TransformComponent* tc, const glm::quat& worldRot);

static float RaySegmentDistance(const glm::vec3& ro, const glm::vec3& rd,
                                const glm::vec3& a, const glm::vec3& b);

static float DistPointToCircle(const glm::vec3& p, const glm::vec3& center, const glm::vec3& n, float r);

// ------------------------------------------------------------
// Local helpers
// ------------------------------------------------------------
static glm::vec3 SafeNormalize(const glm::vec3& v, const glm::vec3& fallback)
{
    float l2 = glm::dot(v, v);
    if (l2 < 1e-10f) return fallback;
    return v / std::sqrt(l2);
}

static glm::vec3 ExtractTranslation(const glm::mat4& m)
{
    return glm::vec3(m[3][0], m[3][1], m[3][2]);
}

static glm::quat ExtractWorldRotation(const glm::mat4& world)
{
    glm::vec3 x = SafeNormalize(glm::vec3(world[0]), glm::vec3(1,0,0));
    glm::vec3 y = SafeNormalize(glm::vec3(world[1]), glm::vec3(0,1,0));
    glm::vec3 z = SafeNormalize(glm::vec3(world[2]), glm::vec3(0,0,1));
    glm::mat3 R(x, y, z);
    return glm::quat_cast(R);
}

static void SetWorldPosition(TransformComponent* tc, const glm::vec3& worldPos)
{
    if (!tc) return;

    GameObject* owner = tc->GetOwner();
    if (!owner) return;

    if (auto parent = owner->GetParent())
    {
        if (auto parentTC = parent->GetComponent<TransformComponent>())
        {
            glm::mat4 invParent = glm::inverse(parentTC->GetWorldTransform());
            glm::vec3 localPos  = glm::vec3(invParent * glm::vec4(worldPos, 1.0f));
            tc->SetPosition(localPos);
            return;
        }
    }

    tc->SetPosition(worldPos);
}

static void SetLocalRotationFromWorld(TransformComponent* tc, const glm::quat& worldRot)
{
    if (!tc) return;

    auto* owner = tc->GetOwner();
    glm::quat parentWorldRot(1, 0, 0, 0);

    if (owner)
    {
        if (auto parent = owner->GetParent())
        {
            if (auto parentTC = parent->GetComponent<TransformComponent>())
                parentWorldRot = ExtractWorldRotation(parentTC->GetWorldTransform());
        }
    }

    glm::quat localRot = glm::inverse(parentWorldRot) * worldRot;
    glm::vec3 eulerRad = glm::eulerAngles(localRot);
    tc->SetRotation(glm::degrees(eulerRad));
}

static float RaySegmentDistance(const glm::vec3& ro, const glm::vec3& rd,
                                const glm::vec3& a, const glm::vec3& b)
{
    glm::vec3 ab = b - a;
    glm::vec3 ao = ro - a;

    float ab2   = glm::dot(ab, ab);
    float rd_ab = glm::dot(rd, ab);
    float rd_ao = glm::dot(rd, ao);
    float ab_ao = glm::dot(ab, ao);

    float denom = ab2 - rd_ab * rd_ab;
    float s = 0.0f;
    float t = 0.0f;

    if (std::abs(denom) > 1e-6f)
    {
        s = (rd_ab * ab_ao - ab2 * rd_ao) / denom;
        t = (rd_ab * s + ab_ao) / ab2;
        t = std::clamp(t, 0.0f, 1.0f);
        s = std::max(s, 0.0f);
    }

    glm::vec3 pRay = ro + rd * s;
    glm::vec3 pSeg = a + ab * t;
    return glm::length(pRay - pSeg);
}

static float DistPointToCircle(const glm::vec3& p, const glm::vec3& center, const glm::vec3& n, float r)
{
    glm::vec3 v = p - center;
    glm::vec3 vPlane = v - n * glm::dot(v, n);
    float len = glm::length(vPlane);
    return std::abs(len - r);
}

float Gizmo::WorldPerPixelAt(const glm::vec3& worldPos) const
{
    auto& eng = Engine::GetInstance();
    auto* cam = eng.renderer ? eng.renderer->renderCamera : nullptr;
    if (!cam) return 0.01f;

    float d = glm::length(cam->GetPosition() - worldPos);

    glm::mat4 P = cam->GetProjectionMatrix();
    float tanHalfFovY = 1.0f / P[1][1];

    // IMPORTANTÍSIMO: usar el viewport del SceneView (framebuffer), no la ventana SDL
    int h = std::max(1, mViewportH);

    float worldScreenHeight = 2.0f * d * tanHalfFovY;
    return worldScreenHeight / (float)h;
}

static void PushTri(std::vector<glm::vec3>& out, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    out.push_back(a); out.push_back(b); out.push_back(c);
}

static void AddThickSegmentBillboard(std::vector<glm::vec3>& out,
                                     const glm::vec3& a,
                                     const glm::vec3& b,
                                     const glm::vec3& camForward,
                                     float halfThicknessWorld)
{
    glm::vec3 dir = SafeNormalize(b - a, glm::vec3(1,0,0));
    glm::vec3 right = SafeNormalize(glm::cross(camForward, dir), glm::vec3(0,1,0));
    glm::vec3 off = right * halfThicknessWorld;

    glm::vec3 v0 = a - off;
    glm::vec3 v1 = a + off;
    glm::vec3 v2 = b + off;
    glm::vec3 v3 = b - off;

    PushTri(out, v0, v1, v2);
    PushTri(out, v0, v2, v3);
}

static void AddArrowHead(std::vector<glm::vec3>& out,
                         const glm::vec3& tip,
                         const glm::vec3& dir,
                         const glm::vec3& camForward,
                         float length,
                         float radius)
{
    glm::vec3 f = SafeNormalize(dir, glm::vec3(1,0,0));
    glm::vec3 right = SafeNormalize(glm::cross(camForward, f), glm::vec3(0,1,0));
    glm::vec3 up    = SafeNormalize(glm::cross(f, right), glm::vec3(0,0,1));

    glm::vec3 baseCenter = tip - f * length;

    glm::vec3 b0 = baseCenter + right * radius;
    glm::vec3 b1 = baseCenter - right * radius;
    glm::vec3 b2 = baseCenter + up * radius;
    glm::vec3 b3 = baseCenter - up * radius;

    PushTri(out, tip, b0, b2);
    PushTri(out, tip, b2, b1);
    PushTri(out, tip, b1, b3);
    PushTri(out, tip, b3, b0);
}

static void AddSquareHandle(std::vector<glm::vec3>& out,
                            const glm::vec3& center,
                            const glm::vec3& camForward,
                            const glm::vec3& axisDir,
                            float halfSize)
{
    glm::vec3 f = SafeNormalize(axisDir, glm::vec3(1,0,0));
    glm::vec3 right = SafeNormalize(glm::cross(camForward, f), glm::vec3(0,1,0));
    glm::vec3 up    = SafeNormalize(glm::cross(f, right), glm::vec3(0,0,1));

    glm::vec3 r = right * halfSize;
    glm::vec3 u = up * halfSize;

    glm::vec3 v0 = center - r - u;
    glm::vec3 v1 = center + r - u;
    glm::vec3 v2 = center + r + u;
    glm::vec3 v3 = center - r + u;

    PushTri(out, v0, v1, v2);
    PushTri(out, v0, v2, v3);
}

static void AddPlaneQuad(std::vector<glm::vec3>& out,
                         const glm::vec3& origin,
                         const glm::vec3& uDir,
                         const glm::vec3& vDir,
                         float u0, float u1,
                         float v0, float v1)
{
    glm::vec3 p00 = origin + uDir * u0 + vDir * v0;
    glm::vec3 p10 = origin + uDir * u1 + vDir * v0;
    glm::vec3 p11 = origin + uDir * u1 + vDir * v1;
    glm::vec3 p01 = origin + uDir * u0 + vDir * v1;

    PushTri(out, p00, p10, p11);
    PushTri(out, p00, p11, p01);
}

// ------------------------------------------------------------
// Gizmo
// ------------------------------------------------------------
std::shared_ptr<GameObject> Gizmo::GetSelected() const
{
    auto scene = Engine::GetInstance().scene;
    return scene ? scene->GetSelected() : nullptr;
}

TransformComponent* Gizmo::GetSelectedTransform() const
{
    auto sel = GetSelected();
    return sel ? sel->GetComponent<TransformComponent>() : nullptr;
}

glm::vec3 Gizmo::GetCameraPosWS() const
{
    auto* cam = Engine::GetInstance().renderer->renderCamera;
    return cam ? cam->GetPosition() : glm::vec3(0);
}

glm::vec3 Gizmo::GetCameraForwardWS() const
{
    auto* cam = Engine::GetInstance().renderer->renderCamera;
    if (!cam) return glm::vec3(0,0,-1);

    glm::mat4 invV = glm::inverse(cam->GetViewMatrix());
    return -SafeNormalize(glm::vec3(invV[2]), glm::vec3(0,0,1));
}

glm::vec3 Gizmo::GetGizmoOriginWS() const
{
    if (auto* tc = GetSelectedTransform())
        return ExtractTranslation(tc->GetWorldTransform());
    return glm::vec3(0);
}

float Gizmo::GetGizmoScaleWS(const glm::vec3& origin) const
{
    float d = glm::length(GetCameraPosWS() - origin);
    return std::max(0.1f, d * 0.12f);
}

glm::vec3 Gizmo::AxisDirWS(GizmoAxis axis) const
{
    glm::vec3 localAxis;
    switch (axis)
    {
        case GizmoAxis::X: localAxis = glm::vec3(1,0,0); break;
        case GizmoAxis::Y: localAxis = glm::vec3(0,1,0); break;
        case GizmoAxis::Z: localAxis = glm::vec3(0,0,1); break;
        default: return glm::vec3(0);
    }

    if (auto* tc = GetSelectedTransform())
    {
        glm::quat q = ExtractWorldRotation(tc->GetWorldTransform());
        return SafeNormalize(q * localAxis, localAxis);
    }

    return localAxis;
}

bool Gizmo::RayPlaneIntersection(const glm::vec3& rayO, const glm::vec3& rayD,
                                 const glm::vec3& planeP, const glm::vec3& planeN,
                                 glm::vec3& outHit) const
{
    float denom = glm::dot(planeN, rayD);
    if (std::abs(denom) < 1e-6f) return false;

    float t = glm::dot(planeP - rayO, planeN) / denom;
    if (t < 0.0f) return false;

    outHit = rayO + rayD * t;
    return true;
}

GizmoAxis Gizmo::PickRotateAxis(float mouseX, float mouseY, const glm::vec3& origin, float /*scale*/) const
{
    auto* scene = Engine::GetInstance().scene.get();
    if (!scene) return GizmoAxis::None;

    auto* cam = Engine::GetInstance().renderer->renderCamera;
    Ray ray = scene->GetRaycaster()->ScreenPointToRay(
        mouseX, mouseY,
        mViewportW, mViewportH
    );

    float wpp    = WorldPerPixelAt(origin);
    float radius = 90.0f * wpp;
    float band   = 12.0f * wpp;  // Increased tolerance for better picking

    GizmoAxis best = GizmoAxis::None;
    float bestD = 1e9f;

    for (auto axis : {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z})
    {
        glm::vec3 n = AxisDirWS(axis);
        
        // Ensure the normal is valid
        if (glm::dot(n, n) < 1e-6f)
            continue;

        glm::vec3 hit;
        // Check if ray intersects the plane perpendicular to the rotation axis
        if (!RayPlaneIntersection(ray.origin, ray.direction, origin, n, hit))
            continue;

        // Calculate distance from hit point to the circle
        float d = DistPointToCircle(hit, origin, n, radius);
        
        // Prioritize circles that are more perpendicular to the camera view
        glm::vec3 camDir = SafeNormalize(origin - GetCameraPosWS(), glm::vec3(0,0,-1));
        float viewAlignment = std::abs(glm::dot(n, camDir));
        
        // Adjust distance based on view alignment (prefer circles facing the camera)
        float adjustedD = d * (1.0f + viewAlignment * 0.5f);
        
        if (d < band && adjustedD < bestD)
        {
            bestD = adjustedD;
            best = axis;
        }
    }

    return best;
}

GizmoAxis Gizmo::PickAxis(float mouseX, float mouseY, const glm::vec3& origin, float /*scale*/) const
{
    auto* scene = Engine::GetInstance().scene.get();
    if (!scene) return GizmoAxis::None;

    auto* cam = Engine::GetInstance().renderer->renderCamera;
    Ray ray = scene->GetRaycaster()->ScreenPointToRay(
        mouseX, mouseY,
        mViewportW, mViewportH
    );

    // 1) Center pick
    {
        glm::vec3 camF = SafeNormalize(GetCameraForwardWS(), glm::vec3(0,0,-1));
        glm::vec3 hit;
        if (RayPlaneIntersection(ray.origin, ray.direction, origin, camF, hit))
        {
            float wpp = WorldPerPixelAt(origin);
            float radius = 10.0f * 0.5f * wpp;
            float band   = 6.0f * wpp;

            if (glm::length(hit - origin) <= (radius + band))
                return GizmoAxis::Center;
        }
    }

    // 2) Plane picks (solo en Translate)
    if (mMode == GizmoMode::Translate)
    {
        float wpp = WorldPerPixelAt(origin);
        float axisLen = 120.0f * wpp;
        float pad = 10.0f * wpp;

        float a0 = 0.25f * axisLen;
        float a1 = 0.55f * axisLen;

        auto pickPlaneQuad = [&](const glm::vec3& u, const glm::vec3& v) -> bool
        {
            glm::vec3 n = SafeNormalize(glm::cross(u, v), glm::vec3(0,0,1));
            glm::vec3 hit;
            if (!RayPlaneIntersection(ray.origin, ray.direction, origin, n, hit))
                return false;

            glm::vec3 d = hit - origin;
            float du = glm::dot(d, u);
            float dv = glm::dot(d, v);

            float uMin = a0 - pad, uMax = a1 + pad;
            float vMin = a0 - pad, vMax = a1 + pad;
            return (du >= uMin && du <= uMax && dv >= vMin && dv <= vMax);
        };

        glm::vec3 X = SafeNormalize(AxisDirWS(GizmoAxis::X), glm::vec3(1,0,0));
        glm::vec3 Y = SafeNormalize(AxisDirWS(GizmoAxis::Y), glm::vec3(0,1,0));
        glm::vec3 Z = SafeNormalize(AxisDirWS(GizmoAxis::Z), glm::vec3(0,0,1));

        if (pickPlaneQuad(X, Y)) return GizmoAxis::PlaneXY;
        if (pickPlaneQuad(X, Z)) return GizmoAxis::PlaneXZ;
        if (pickPlaneQuad(Y, Z)) return GizmoAxis::PlaneYZ;
    }

    // 3) Axis picks X/Y/Z
    {
        float wpp = WorldPerPixelAt(origin);
        float axisLen = 120.0f * wpp;
        float threshold = 10.0f * wpp;

        GizmoAxis best = GizmoAxis::None;
        float bestDist = 1e9f;

        for (auto axis : {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z})
        {
            glm::vec3 dir = AxisDirWS(axis);
            glm::vec3 a = origin;
            glm::vec3 b = origin + dir * axisLen;

            float d = RaySegmentDistance(ray.origin, ray.direction, a, b);
            if (d < threshold && d < bestDist)
            {
                bestDist = d;
                best = axis;
            }
        }

        return best;
    }
}

void Gizmo::Update(float mouseX, float mouseY)
{
    if (!GetSelectedTransform())
    {
        mHoveredAxis = GizmoAxis::None;
        mDragging = false;
        mActiveAxis = GizmoAxis::None;
        return;
    }

    if (mMode == GizmoMode::None)
    {
        mHoveredAxis = GizmoAxis::None;
        mDragging = false;
        mActiveAxis = GizmoAxis::None;
        return;
    }

    if (!mDragging)
    {
        glm::vec3 origin = GetGizmoOriginWS();
        float scale = GetGizmoScaleWS(origin);

        if (mMode == GizmoMode::Rotate)
            mHoveredAxis = PickRotateAxis(mouseX, mouseY, origin, scale);
        else
            mHoveredAxis = PickAxis(mouseX, mouseY, origin, scale);
    }

    // ---------- TRANSLATE ----------
    if (mDragging && mMode == GizmoMode::Translate && mActiveAxis != GizmoAxis::None)
    {
        auto* tc = GetSelectedTransform();
        if (!tc) return;

        auto* scene = Engine::GetInstance().scene.get();
        auto* cam = Engine::GetInstance().renderer->renderCamera;
        Ray ray = scene->GetRaycaster()->ScreenPointToRay(
            mouseX, mouseY,
            mViewportW, mViewportH
        );

        glm::vec3 planeP = mDragStartHitWS;
        glm::vec3 planeN = mTranslatePlaneN;

        glm::vec3 hit;
        if (!RayPlaneIntersection(ray.origin, ray.direction, planeP, planeN, hit))
            return;

        glm::vec3 currentWorldPos = ExtractTranslation(tc->GetWorldTransform());
        glm::vec3 newWorldPos = currentWorldPos;

        if (mActiveAxis == GizmoAxis::Center)
        {
            glm::vec3 d = hit - mDragStartHitWS;
            newWorldPos = currentWorldPos + d;
        }
        else if (mActiveAxis == GizmoAxis::PlaneXY || mActiveAxis == GizmoAxis::PlaneXZ || mActiveAxis == GizmoAxis::PlaneYZ)
        {
            glm::vec3 d = hit - mDragStartHitWS;
            newWorldPos = currentWorldPos + d;

            // Snap opcional en plano (Ctrl)
            auto* input = Engine::GetInstance().input.get();
            bool snap = input && (
                input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT ||
                input->GetKey(SDL_SCANCODE_RCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT
            );

            if (snap)
            {
                float step = mTranslateSnap;

                glm::vec3 X = SafeNormalize(AxisDirWS(GizmoAxis::X), glm::vec3(1,0,0));
                glm::vec3 Y = SafeNormalize(AxisDirWS(GizmoAxis::Y), glm::vec3(0,1,0));
                glm::vec3 Z = SafeNormalize(AxisDirWS(GizmoAxis::Z), glm::vec3(0,0,1));

                glm::vec3 U, V;
                if (mActiveAxis == GizmoAxis::PlaneXY) { U = X; V = Y; }
                if (mActiveAxis == GizmoAxis::PlaneXZ) { U = X; V = Z; }
                if (mActiveAxis == GizmoAxis::PlaneYZ) { U = Y; V = Z; }

                float u = glm::dot(newWorldPos, U);
                float v = glm::dot(newWorldPos, V);

                u = std::round(u / step) * step;
                v = std::round(v / step) * step;

                glm::vec3 perp = newWorldPos - U * glm::dot(newWorldPos, U) - V * glm::dot(newWorldPos, V);
                newWorldPos = perp + U * u + V * v;
            }
        }
        else
        {
            glm::vec3 axisDir = AxisDirWS(mActiveAxis);
            float delta = glm::dot(hit - mDragStartHitWS, axisDir);
            newWorldPos = currentWorldPos + axisDir * delta;

            // Ctrl snapping (por eje)
            auto* input = Engine::GetInstance().input.get();
            bool snap = input && (
                input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT ||
                input->GetKey(SDL_SCANCODE_RCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT
            );

            if (snap)
            {
                float step = mTranslateSnap;

                float v = glm::dot(newWorldPos, axisDir);
                v = std::round(v / step) * step;

                glm::vec3 perp = newWorldPos - axisDir * glm::dot(newWorldPos, axisDir);
                newWorldPos = perp + axisDir * v;
            }
        }

        SetWorldPosition(tc, newWorldPos);
        mDragStartHitWS = hit;
    }

    // ---------- ROTATE ----------
    if (mDragging && mMode == GizmoMode::Rotate && mActiveAxis != GizmoAxis::None)
    {
        auto* tc = GetSelectedTransform();
        if (!tc) return;

        glm::vec3 origin = GetGizmoOriginWS();

        auto* scene = Engine::GetInstance().scene.get();
        auto* cam = Engine::GetInstance().renderer->renderCamera;
        Ray ray = scene->GetRaycaster()->ScreenPointToRay(
            mouseX, mouseY,
            mViewportW, mViewportH
        );

        glm::vec3 hit;
        if (!RayPlaneIntersection(ray.origin, ray.direction, origin, mRotatePlaneN, hit))
            return;

        glm::vec3 v = hit - origin;
        if (glm::dot(v, v) < 1e-10f) return;
        v = glm::normalize(v);

        float angle = std::atan2(
            glm::dot(glm::cross(mRotateStartVec, v), mRotatePlaneN),
            glm::dot(mRotateStartVec, v)
        );

        // Ctrl snap 15°
        auto* input = Engine::GetInstance().input.get();
        bool snap = input && (
            input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT ||
            input->GetKey(SDL_SCANCODE_RCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT
        );
        if (snap)
        {
            float step = glm::radians(15.0f);
            angle = std::round(angle / step) * step;
        }

        glm::quat dq = glm::angleAxis(angle, SafeNormalize(mRotatePlaneN, glm::vec3(0,1,0)));
        glm::quat newWorld = dq * mRotateStartWorld;

        SetLocalRotationFromWorld(tc, newWorld);
    }

    // ---------- SCALE ----------
    if (mDragging && mMode == GizmoMode::Scale && mActiveAxis != GizmoAxis::None)
    {
        auto* tc = GetSelectedTransform();
        if (!tc) return;

        auto* scene = Engine::GetInstance().scene.get();
        auto* cam = Engine::GetInstance().renderer->renderCamera;
        Ray ray = scene->GetRaycaster()->ScreenPointToRay(
            mouseX, mouseY,
            mViewportW, mViewportH
        );

        glm::vec3 origin = GetGizmoOriginWS();
        glm::vec3 planeN = GetCameraForwardWS();

        glm::vec3 hit;
        if (!RayPlaneIntersection(ray.origin, ray.direction, mDragStartHitWS, planeN, hit))
            return;

        auto* input = Engine::GetInstance().input.get();
        bool snap = input && (
            input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT ||
            input->GetKey(SDL_SCANCODE_RCTRL) == KEY_DOWN || input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT
        );

        if (mActiveAxis == GizmoAxis::Center)
        {
            float curDist = std::max(1e-4f, glm::length(hit - origin));
            float ratio   = curDist / std::max(1e-4f, mUniformScaleStartDist);

            glm::vec3 newS = mScaleStartLocal * ratio;
            newS = glm::max(newS, glm::vec3(0.001f));

            if (snap)
            {
                float step = 0.1f;
                newS.x = std::round(newS.x / step) * step;
                newS.y = std::round(newS.y / step) * step;
                newS.z = std::round(newS.z / step) * step;
                newS = glm::max(newS, glm::vec3(0.001f));
            }

            tc->SetScale(newS);

            mUniformScaleStartDist = curDist;
            mScaleStartLocal = newS;
            mDragStartHitWS = hit;
        }
        else
        {
            glm::vec3 axisDir = AxisDirWS(mActiveAxis);
            float delta = glm::dot(hit - mDragStartHitWS, axisDir);

            float factor = 1.0f + (delta / (GetGizmoScaleWS(origin) * 1.0f));
            factor = std::max(0.01f, factor);

            glm::vec3 s = tc->GetScale();
            if (mActiveAxis == GizmoAxis::X) s.x = std::max(0.001f, mScaleStartLocal.x * factor);
            if (mActiveAxis == GizmoAxis::Y) s.y = std::max(0.001f, mScaleStartLocal.y * factor);
            if (mActiveAxis == GizmoAxis::Z) s.z = std::max(0.001f, mScaleStartLocal.z * factor);

            if (snap)
            {
                float step = 0.1f;
                s.x = std::round(s.x / step) * step;
                s.y = std::round(s.y / step) * step;
                s.z = std::round(s.z / step) * step;
                s = glm::max(s, glm::vec3(0.001f));
            }

            tc->SetScale(s);
        }
    }
}

bool Gizmo::OnMouseDown(float mouseX, float mouseY)
{
    if (!GetSelectedTransform()) return false;
    if (mMode == GizmoMode::None) return false;
    if (mHoveredAxis == GizmoAxis::None) return false;

    mDragging = true;
    mActiveAxis = mHoveredAxis;

    auto* scene = Engine::GetInstance().scene.get();
    auto* cam = Engine::GetInstance().renderer->renderCamera;
    Ray ray = scene->GetRaycaster()->ScreenPointToRay(
        mouseX, mouseY,
        mViewportW, mViewportH
    );

    glm::vec3 origin = GetGizmoOriginWS();

    // ROTATE init
    if (mMode == GizmoMode::Rotate)
    {
        auto* tc = GetSelectedTransform();
        if (!tc) return false;

        mRotatePlaneN = SafeNormalize(AxisDirWS(mActiveAxis), glm::vec3(0,1,0));

        glm::vec3 hit;
        if (!RayPlaneIntersection(ray.origin, ray.direction, origin, mRotatePlaneN, hit))
        {
            mDragging = false;
            mActiveAxis = GizmoAxis::None;
            return false;
        }

        glm::vec3 v = hit - origin;
        if (glm::dot(v, v) < 1e-10f)
        {
            mDragging = false;
            mActiveAxis = GizmoAxis::None;
            return false;
        }

        mRotateStartVec   = glm::normalize(v);
        mRotateStartWorld = ExtractWorldRotation(tc->GetWorldTransform());
        return true;
    }

    // TRANSLATE / SCALE init
    glm::vec3 planeN = GetCameraForwardWS();

    if (mMode == GizmoMode::Translate)
    {
        glm::vec3 X = SafeNormalize(AxisDirWS(GizmoAxis::X), glm::vec3(1,0,0));
        glm::vec3 Y = SafeNormalize(AxisDirWS(GizmoAxis::Y), glm::vec3(0,1,0));
        glm::vec3 Z = SafeNormalize(AxisDirWS(GizmoAxis::Z), glm::vec3(0,0,1));

        if (mActiveAxis == GizmoAxis::PlaneXY) planeN = SafeNormalize(glm::cross(X, Y), glm::vec3(0,0,1));
        if (mActiveAxis == GizmoAxis::PlaneXZ) planeN = SafeNormalize(glm::cross(X, Z), glm::vec3(0,1,0));
        if (mActiveAxis == GizmoAxis::PlaneYZ) planeN = SafeNormalize(glm::cross(Y, Z), glm::vec3(1,0,0));

        if (mActiveAxis == GizmoAxis::Center) planeN = GetCameraForwardWS();

        mTranslatePlaneN = planeN;
    }

    glm::vec3 hit;
    if (!RayPlaneIntersection(ray.origin, ray.direction, origin, planeN, hit))
        hit = origin;

    mDragStartHitWS = hit;

    // SCALE init
    if (mMode == GizmoMode::Scale)
    {
        if (auto* tc = GetSelectedTransform())
            mScaleStartLocal = tc->GetScale();

        if (mActiveAxis == GizmoAxis::Center)
            mUniformScaleStartDist = std::max(1e-4f, glm::length(mDragStartHitWS - origin));
    }

    return true;
}

void Gizmo::OnMouseUp()
{
    mDragging = false;
    mActiveAxis = GizmoAxis::None;
}

void Gizmo::Render()
{
    if (mMode == GizmoMode::None) return;
    if (!GetSelectedTransform()) return;

    glm::vec3 origin = GetGizmoOriginWS();

    auto shader = Renderer::GetSingleColorShader();
    if (!shader) return;

    auto* cam = Engine::GetInstance().renderer->renderCamera;
    if (!cam) return;

    static GLuint vao = 0, vbo = 0;
    if (!vao)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    shader->Bind();
    shader->SetMat4("view", cam->GetViewMatrix());
    shader->SetMat4("projection", cam->GetProjectionMatrix());
    shader->SetMat4("model", glm::mat4(1.0f));
    shader->SetFloat("outlineThickness", 0.0f);

    auto axisColor = [&](GizmoAxis axis)
    {
        switch (axis)
        {
            case GizmoAxis::X: return glm::vec3(1,0,0);
            case GizmoAxis::Y: return glm::vec3(0,1,0);
            case GizmoAxis::Z: return glm::vec3(0,0,1);
            default: return glm::vec3(1,1,1);
        }
    };

    auto highlight = [&](GizmoAxis axis, glm::vec3 c)
    {
        // Si el eje está siendo arrastrado (activo), hacerlo más oscuro
        if (axis == mActiveAxis && mDragging)
        {
            c *= 0.5f; // Más oscuro durante el drag
        }
        // Si solo está hover, hacerlo más brillante
        else if (axis == mHoveredAxis)
        {
            c *= 1.5f; // Más brillante en hover
        }
        return c;
    };

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    float wpp = WorldPerPixelAt(origin);
    glm::vec3 camF = SafeNormalize(GetCameraForwardWS(), glm::vec3(0,0,-1));

    // ROTATE rings
    if (mMode == GizmoMode::Rotate)
    {
        float radius = 90.0f * wpp;

        auto drawCircle = [&](GizmoAxis axis)
        {
            glm::vec3 n = AxisDirWS(axis);

            glm::vec3 a = (std::abs(n.x) < 0.9f) ? glm::vec3(1,0,0) : glm::vec3(0,1,0);
            glm::vec3 u = SafeNormalize(glm::cross(n, a), glm::vec3(1,0,0));
            glm::vec3 v = SafeNormalize(glm::cross(n, u), glm::vec3(0,1,0));

            const int N = 64;
            std::vector<glm::vec3> pts;
            pts.reserve(N + 1);

            for (int i = 0; i <= N; i++)
            {
                float t = (float)i / (float)N * glm::two_pi<float>();
                glm::vec3 p = origin + (u * std::cos(t) + v * std::sin(t)) * radius;
                pts.push_back(p);
            }

            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(pts.size() * sizeof(glm::vec3)), pts.data(), GL_DYNAMIC_DRAW);
            shader->SetVec3("outlineColor", highlight(axis, axisColor(axis)));
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)pts.size());
        };

        drawCircle(GizmoAxis::X);
        drawCircle(GizmoAxis::Y);
        drawCircle(GizmoAxis::Z);

        if (depthWasEnabled) glEnable(GL_DEPTH_TEST);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
        return;
    }

    // TRANSLATE / SCALE
    {
        float shaftPx = 6.0f;
        float headPx  = 14.0f;

        float shaftHalf  = (shaftPx * 0.5f) * wpp;
        float handleHalf = (headPx  * 0.5f) * wpp;

        float axisLen = 120.0f * wpp;
        float headLen = 22.0f  * wpp;
        float headRad = 12.0f  * wpp;

        static std::vector<glm::vec3> verts;
        verts.reserve(1024);

        auto drawAxis = [&](GizmoAxis axis)
        {
            glm::vec3 dir = AxisDirWS(axis);
            glm::vec3 a = origin;
            glm::vec3 b = origin + dir * axisLen;

            verts.clear();
            AddThickSegmentBillboard(verts, a, b, camF, shaftHalf);

            if (mMode == GizmoMode::Translate)
                AddArrowHead(verts, b, dir, camF, headLen, headRad);
            else if (mMode == GizmoMode::Scale)
                AddSquareHandle(verts, b, camF, dir, handleHalf);

            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(glm::vec3)), verts.data(), GL_DYNAMIC_DRAW);
            shader->SetVec3("outlineColor", highlight(axis, axisColor(axis)));
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
        };

        drawAxis(GizmoAxis::X);
        drawAxis(GizmoAxis::Y);
        drawAxis(GizmoAxis::Z);

        // Center handle
        {
            float centerHalf = (10.0f * 0.5f) * wpp;

            verts.clear();
            AddSquareHandle(verts, origin, camF, camF, centerHalf);

            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(glm::vec3)), verts.data(), GL_DYNAMIC_DRAW);

            glm::vec3 c = glm::vec3(1.0f, 1.0f, 0.2f);
            if (mHoveredAxis == GizmoAxis::Center || mActiveAxis == GizmoAxis::Center) c *= 1.5f;

            shader->SetVec3("outlineColor", c);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
        }

        // Plane handles (solo Translate)
        if (mMode == GizmoMode::Translate)
        {
            float a0 = 0.25f * axisLen;
            float a1 = 0.55f * axisLen;

            glm::vec3 X = SafeNormalize(AxisDirWS(GizmoAxis::X), glm::vec3(1,0,0));
            glm::vec3 Y = SafeNormalize(AxisDirWS(GizmoAxis::Y), glm::vec3(0,1,0));
            glm::vec3 Z = SafeNormalize(AxisDirWS(GizmoAxis::Z), glm::vec3(0,0,1));

            auto drawPlane = [&](GizmoAxis planeAxis, const glm::vec3& u, const glm::vec3& v, const glm::vec3& color)
            {
                verts.clear();
                AddPlaneQuad(verts, origin, u, v, a0, a1, a0, a1);

                glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(glm::vec3)), verts.data(), GL_DYNAMIC_DRAW);

                glm::vec3 c = color;
                if (mHoveredAxis == planeAxis || mActiveAxis == planeAxis) c *= 1.5f;

                shader->SetVec3("outlineColor", c);
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
            };

            drawPlane(GizmoAxis::PlaneXY, X, Y, glm::vec3(1,1,0));
            drawPlane(GizmoAxis::PlaneXZ, X, Z, glm::vec3(1,0,1));
            drawPlane(GizmoAxis::PlaneYZ, Y, Z, glm::vec3(0,1,1));
        }
    }

    if (depthWasEnabled) glEnable(GL_DEPTH_TEST);

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}
#include "Gizmo.h"

#include "Engine.h"
#include "ModuleScene.h"
#include "ModuleRenderer.h"
#include "Raycaster.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

static void SetWorldPosition(TransformComponent* tc, const glm::vec3& worldPos)
{
    if (!tc) return;

    GameObject* owner = tc->GetOwner();
    if (!owner) return;

    // Si tiene padre: localPos = inverse(parentWorld) * worldPos
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

    // Sin padre -> world == local
    tc->SetPosition(worldPos);
}

static glm::vec3 ExtractTranslation(const glm::mat4& m)
{
    return glm::vec3(m[3][0], m[3][1], m[3][2]);
}

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

    // forward desde view matrix
    glm::mat4 invV = glm::inverse(cam->GetViewMatrix());
    glm::vec3 forward = -glm::normalize(glm::vec3(invV[2])); // -Z
    return forward;
}

glm::vec3 Gizmo::GetGizmoOriginWS() const
{
    if (auto* tc = GetSelectedTransform())
    {
        // OJO: tu Transform guarda local, pero world matrix ya está listo
        return ExtractTranslation(tc->GetWorldTransform());
    }
    return glm::vec3(0);
}

float Gizmo::GetGizmoScaleWS(const glm::vec3& origin) const
{
    float d = glm::length(GetCameraPosWS() - origin);
    return std::max(0.1f, d * 0.12f); // “feel” tipo Unity
}

glm::vec3 Gizmo::AxisDirWS(GizmoAxis axis) const
{
    switch (axis)
    {
        case GizmoAxis::X: return glm::vec3(1,0,0);
        case GizmoAxis::Y: return glm::vec3(0,1,0);
        case GizmoAxis::Z: return glm::vec3(0,0,1);
        default: return glm::vec3(0);
    }
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

static float RaySegmentDistance(const glm::vec3& ro, const glm::vec3& rd,
                                const glm::vec3& a, const glm::vec3& b)
{
    // distancia mínima entre rayo y segmento (aprox robusta)
    glm::vec3 ab = b - a;
    glm::vec3 ao = ro - a;

    float ab2 = glm::dot(ab, ab);
    float rd_ab = glm::dot(rd, ab);
    float rd_ao = glm::dot(rd, ao);
    float ab_ao = glm::dot(ab, ao);

    float denom = ab2 - rd_ab * rd_ab;
    float s = 0.0f;
    float t = 0.0f;

    if (std::abs(denom) > 1e-6f)
    {
        s = (rd_ab * ab_ao - ab2 * rd_ao) / denom; // rayo
        t = (rd_ab * s + ab_ao) / ab2;            // segmento
        t = std::clamp(t, 0.0f, 1.0f);
        s = std::max(s, 0.0f);
    }

    glm::vec3 pRay = ro + rd * s;
    glm::vec3 pSeg = a + ab * t;

    return glm::length(pRay - pSeg);
}

GizmoAxis Gizmo::PickAxis(float mouseX, float mouseY, const glm::vec3& origin, float scale) const
{
    auto* scene = Engine::GetInstance().scene.get();
    if (!scene) return GizmoAxis::None;

    Ray ray = scene->GetRaycaster()->ScreenPointToRay(mouseX, mouseY);

    const float axisLen = 1.0f * scale;
    const float threshold = 0.07f * scale; // tolerancia en mundo

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

void Gizmo::Update(float mouseX, float mouseY)
{
    if (!GetSelectedTransform())
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
        mHoveredAxis = PickAxis(mouseX, mouseY, origin, scale);
    }

    // Dragging: aplicar translate
    if (mDragging && mMode == GizmoMode::Translate && mActiveAxis != GizmoAxis::None)
    {
        auto* scene = Engine::GetInstance().scene.get();
        Ray ray = scene->GetRaycaster()->ScreenPointToRay(mouseX, mouseY);

        glm::vec3 planeP = mDragStartHitWS;
        glm::vec3 planeN = GetCameraForwardWS(); // plano “screen-facing”

        glm::vec3 hit;
        if (RayPlaneIntersection(ray.origin, ray.direction, planeP, planeN, hit))
        {
            glm::vec3 axisDir = AxisDirWS(mActiveAxis);
            float delta = glm::dot(hit - mDragStartHitWS, axisDir);

            // IMPORTANTE: tu TransformComponent SetPosition es LOCAL.
            // Por ahora: si no hay padre, world==local y esto funciona.
            // En el siguiente paso lo hacemos bien para jerarquías.
            if (auto* tc = GetSelectedTransform())
            {
                glm::vec3 currentWorldPos = ExtractTranslation(tc->GetWorldTransform());
                glm::vec3 newWorldPos     = currentWorldPos + axisDir * delta;

                SetWorldPosition(tc, newWorldPos);

                // seguir incremental para buen “feeling”
                mDragStartHitWS = hit;
            }
        }
    }
}

bool Gizmo::OnMouseDown(float mouseX, float mouseY)
{
    if (!GetSelectedTransform()) return false;

    if (mHoveredAxis == GizmoAxis::None) return false;

    mDragging = true;
    mActiveAxis = mHoveredAxis;

    // guardar estado inicial
    if (auto* tc = GetSelectedTransform())
        mDragStartPosWS = ExtractTranslation(tc->GetWorldTransform());

    // punto inicial del drag en el plano
    auto* scene = Engine::GetInstance().scene.get();
    Ray ray = scene->GetRaycaster()->ScreenPointToRay(mouseX, mouseY);

    glm::vec3 origin = GetGizmoOriginWS();
    glm::vec3 planeN = GetCameraForwardWS();
    glm::vec3 hit;
    if (!RayPlaneIntersection(ray.origin, ray.direction, origin, planeN, hit))
        hit = origin;

    mDragStartHitWS = hit;
    return true;
}

void Gizmo::OnMouseUp()
{
    mDragging = false;
    mActiveAxis = GizmoAxis::None;
}

void Gizmo::Render()
{
    if (!GetSelectedTransform()) return;

    glm::vec3 origin = GetGizmoOriginWS();
    float scale = GetGizmoScaleWS(origin);
    float axisLen = 1.0f * scale;

    auto shader = Renderer::GetSingleColorShader();
    if (!shader) return;

    // Dibujar 3 líneas (como tus DebugDrawAABB/Ray)
    // Reutilizamos un VAO/VBO simple para 2 puntos
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

    auto* cam = Engine::GetInstance().renderer->renderCamera;
    shader->Bind();
    shader->SetMat4("view", cam->GetViewMatrix());
    shader->SetMat4("projection", cam->GetProjectionMatrix());
    shader->SetMat4("model", glm::mat4(1.0f));
    shader->SetFloat("outlineThickness", 0.0f);

    auto drawAxis = [&](GizmoAxis axis, const glm::vec3& color)
    {
        glm::vec3 dir = AxisDirWS(axis);
        glm::vec3 pts[2] = { origin, origin + dir * axisLen };

        glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_DYNAMIC_DRAW);

        // si está hovered/active, lo “iluminamos”
        glm::vec3 c = color;
        if (axis == mHoveredAxis || axis == mActiveAxis)
            c *= 1.5f;

        shader->SetVec3("outlineColor", c);
        glDrawArrays(GL_LINES, 0, 2);
    };

    drawAxis(GizmoAxis::X, glm::vec3(1,0,0));
    drawAxis(GizmoAxis::Y, glm::vec3(0,1,0));
    drawAxis(GizmoAxis::Z, glm::vec3(0,0,1));

    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}
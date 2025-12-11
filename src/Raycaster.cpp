#include "Raycaster.h"
#include "Engine.h"
#include "Globals.h"
#include "ModuleWindow.h"
#include "ModuleRenderer.h"
#include "TransformComponent.h"
#include "ModuleScene.h"   
#include "Quadtree.h"

Raycaster::Raycaster()
{
}

Ray Raycaster::ScreenPointToRay(float mouseX, float mouseY) const
{
    ICamera* camera = Engine::GetInstance().renderer->renderCamera;
    
    int width, height;
    SDL_GetWindowSizeInPixels(
        Engine::GetInstance().window->GetWindow(),
        &width, &height);

    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;

    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 view       = camera->GetViewMatrix();

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

    Ray ray{ camera->GetPosition(), rayDir };

    mLastRay   = ray;
    mHasLastRay = true;

    return ray;
}

std::shared_ptr<GameObject> Raycaster::PickObject(float mouseX, float mouseY, const std::vector<std::shared_ptr<GameObject>> objects) const
{
    Ray ray = ScreenPointToRay(mouseX, mouseY);

    std::shared_ptr<GameObject> closest = nullptr;
    float closestT = std::numeric_limits<float>::max();

    // --- NUEVO: usar quadtree si está disponible ---
    std::vector<std::shared_ptr<GameObject>> candidates;

    ModuleScene* scene = Engine::GetInstance().scene.get();
    if (scene && scene->GetQuadtree())
    {
        scene->GetQuadtree()->QueryRay(ray, candidates);
    }
    else
    {
        // Fallback: usar la lista completa (como antes)
        candidates = objects;
    }

    for (std::shared_ptr<GameObject> obj : candidates)
    {
        MeshComponent *meshComp = obj->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->GetMesh())
            continue;

        const AABB &localBox = meshComp->GetMesh()->GetAABB();

        // Transform AABB to world space
        AABB worldBox = localBox;
        if (auto tc = obj->GetComponent<TransformComponent>())
        {
            glm::mat4 worldTransform = tc->GetWorldTransform();
            worldBox = TransformAABB(localBox, worldTransform); // usa el de AABB.h
        }

        float tHit;
        if (RayIntersectsAABB(ray, worldBox, &tHit))
        {
            if (tHit < closestT)
            {
                closestT = tHit;
                closest = obj;
            }
        }
    }

    return closest;
}

bool Raycaster::RayIntersectsAABB(const Ray &ray, const AABB &box, float *tMinOut)
{
    glm::vec3 t0 = (box.min - ray.origin) / ray.direction;
    glm::vec3 t1 = (box.max - ray.origin) / ray.direction;

    glm::vec3 tMinVec = glm::min(t0, t1);
    glm::vec3 tMaxVec = glm::max(t0, t1);

    float tMin = std::max({tMinVec.x, tMinVec.y, tMinVec.z});
    float tMax = std::min({tMaxVec.x, tMaxVec.y, tMaxVec.z});

    bool hit = tMax >= std::max(tMin, 0.0f);
    if (hit && tMinOut)
        *tMinOut = tMin;

    return hit;
}

AABB Raycaster::TransformAABB(const AABB &aabb, const glm::mat4 &transform)
{
    glm::vec3 corners[8] = {
        glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z)
    };

    glm::vec3 newMin = glm::vec3(transform * glm::vec4(corners[0], 1.0f));
    glm::vec3 newMax = newMin;

    for (int i = 1; i < 8; ++i)
    {
        glm::vec3 transformed = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
        newMin = glm::min(newMin, transformed);
        newMax = glm::max(newMax, transformed);
    }

    return AABB(newMin, newMax);
}

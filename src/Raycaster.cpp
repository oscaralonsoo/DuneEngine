#include "Raycaster.h"
#include "Engine.h"
#include "Globals.h"
#include "ModuleWindow.h"
#include "ModuleRenderer.h"

Raycaster::Raycaster()
{
}

Ray Raycaster::ScreenPointToRay(float mouseX, float mouseY) const
{
    EditorCamera* camera = Engine::GetInstance().renderer.get()->renderCamera;
    
    int width, height;
    SDL_GetWindowSizeInPixels(Engine::GetInstance().window.get()->GetWindow(), &width, &height);

    float aspect = (height > 0) ? (float)width / (float)height : 4.0f / 3.0f;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;

    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    glm::mat4 view = camera->GetViewMatrix();
    glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

    return Ray{camera->GetPosition(), rayDir};
}

GameObject *Raycaster::PickObject(float mouseX, float mouseY, const std::vector<GameObject *> &objects) const
{
    Ray ray = ScreenPointToRay(mouseX, mouseY);

    GameObject *closest = nullptr;
    float closestT = std::numeric_limits<float>::max();

    for (auto *obj : objects)
    {
        MeshComponent *meshComp = obj->GetComponent<MeshComponent>();
        if (!meshComp)
            continue;

        const AABB &box = meshComp->GetMesh()->GetAABB();

        float tHit;
        if (RayIntersectsAABB(ray, box, &tHit))
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

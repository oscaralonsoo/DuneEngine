#pragma once

#include "Camera.h"
#include "MeshComponent.h"
#include "Mesh.h"
#include "GameObject.h"
#include "AABB.h"

#include <vector>
#include <glm/glm.hpp>
#include <limits>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3 &o, const glm::vec3 &d)
        : origin(o), direction(d) {}
};

class Raycaster
{
public:
    Raycaster();
    ~Raycaster() = default;

    Ray ScreenPointToRay(float mouseX, float mouseY) const;

    std::shared_ptr<GameObject> PickObject(float mouseX, float mouseY, const std::vector<std::shared_ptr<GameObject>> objects) const;

private:
    static bool RayIntersectsAABB(const Ray &ray, const AABB &box, float *tMinOut = nullptr);

private:
    Camera *camera;
    int screenWidth, screenHeight;
};

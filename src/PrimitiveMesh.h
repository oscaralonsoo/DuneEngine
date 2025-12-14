#pragma once
#include "Mesh.h"

enum class PrimitiveType
{
    Quad,
    Cube,
    Sphere,
    Plane,
    Cylinder,
    Cone,
    Torus,
    Capsule,
    None
};

class PrimitiveMesh
{
public:
    PrimitiveMesh();
    ~PrimitiveMesh() = default;

    static std::shared_ptr<Mesh> CreateQuad();
    static std::shared_ptr<Mesh> CreateCube(const glm::vec3 &size = {1.0f, 1.0f, 1.0f}, int subdivideW = 0, int subdidiveH = 0, int subdivideD = 0);
    static std::shared_ptr<Mesh> CreateSphere(float radius = 0.5f, float height = 1.0f, int radialSegments = 64, int rings = 32, bool isHemiSphere = false);
    static std::shared_ptr<Mesh> CreatePlane(const glm::vec2& size = { 1.0f, 1.0f }, const glm::vec3& normal = { 0.0f, 1.0f, 0.0f });
    static std::shared_ptr<Mesh> CreateCylinder(float topRadius = 0.5f, float bottomRadius = 0.5f, float height = 1.0f, int radialSegments = 64, int rings = 1, bool capTop = true, bool capBottom = true);
    static std::shared_ptr<Mesh> CreateCone(float radius = 0.5f, float height = 1.0f, int radialSegments = 64, int rings = 1, bool cap = true);
    static std::shared_ptr<Mesh> CreateTorus(float innerRadius = 0.5f, float outerRadius = 1.0f, int rings = 64, int ringSegments = 32);
    static std::shared_ptr<Mesh> CreateCapsule(float radius = 0.5f, float height = 2.0f, int radialSegments = 64, int rings = 8);

private:
};

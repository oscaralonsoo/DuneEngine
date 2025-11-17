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

    //static std::shared_ptr<Mesh> CreateQuad();
    static std::shared_ptr<Mesh> CreateCube(const glm::vec3 &size = {1.0f, 1.0f, 1.0f}, int subdivideW = 0, int subdidiveH = 0, int subdivideD = 0);
    static  std::shared_ptr<Mesh> CreateSphere(float radius = 0.5f, float height = 1.0f, int radialSegments = 64, int rings = 32, bool isHemiSphere = false);

private:
};

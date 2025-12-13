#pragma once

#include <memory>
#include <vector>
#include "AABB.h"
#include "GameObject.h"

// Forward declarations para evitar includes circulares
struct Ray;
class Frustum;

// Quadtree estático de GameObjects (solo para broad-phase)
class Quadtree
{
public:
    Quadtree(const AABB& bounds, int maxDepth = 6, int maxObjectsPerNode = 4);

    // Reestablece el árbol con nuevos límites (borra todos los nodos/objetos)
    void SetBounds(const AABB& bounds);

    // Inserta un objeto con su AABB en mundo
    void Insert(const std::shared_ptr<GameObject>& object, const AABB& worldAABB);

    // Query por rayo: devuelve posibles candidatos a colisión
    void QueryRay(const Ray& ray, std::vector<std::shared_ptr<GameObject>>& outObjects) const;

    // Query por frustum: devuelve objetos cuyo AABB intersecta el frustum
    void QueryFrustum(const Frustum& frustum, std::vector<std::shared_ptr<GameObject>>& outObjects) const;

    const AABB& GetBounds() const;

private:
    struct Node
    {
        AABB bounds;
        // Guardamos objeto + su AABB en mundo
        std::vector<std::pair<std::shared_ptr<GameObject>, AABB>> objects;
        std::unique_ptr<Node> children[4]; // subdivisión en XZ

        Node(const AABB& b) : bounds(b) {}
        bool IsLeaf() const { return children[0] == nullptr; }
    };

    std::unique_ptr<Node> mRoot;
    int mMaxDepth;
    int mMaxObjectsPerNode;

    void Subdivide(Node* node);
    int GetChildIndex(const Node* node, const AABB& box) const;

    void InsertRecursive(Node* node,
                         const std::shared_ptr<GameObject>& object,
                         const AABB& box,
                         int depth);

    void QueryRayRecursive(const Node* node,
                           const Ray& ray,
                           std::vector<std::shared_ptr<GameObject>>& outObjects) const;

    void QueryFrustumRecursive(const Node* node,
                               const Frustum& frustum,
                               std::vector<std::shared_ptr<GameObject>>& outObjects) const;

    static bool RayIntersectsAABB(const Ray& ray,
                                  const AABB& box,
                                  float* tMinOut = nullptr);
};
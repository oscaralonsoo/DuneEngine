#include "Quadtree.h"
#include "Raycaster.h"
#include "Frustum.h"

Quadtree::Quadtree(const AABB& bounds, int maxDepth, int maxObjectsPerNode)
    : mMaxDepth(maxDepth)
    , mMaxObjectsPerNode(maxObjectsPerNode)
{
    mRoot = std::make_unique<Node>(bounds);
}

void Quadtree::SetBounds(const AABB& bounds)
{
    mRoot = std::make_unique<Node>(bounds);
}

const AABB& Quadtree::GetBounds() const
{
    return mRoot->bounds;
}

void Quadtree::Insert(const std::shared_ptr<GameObject>& object, const AABB& worldAABB)
{
    if (!mRoot) return;
    InsertRecursive(mRoot.get(), object, worldAABB, 0);
}

void Quadtree::Subdivide(Node* node)
{
    const glm::vec3 center = node->bounds.GetCenter();
    const glm::vec3 min    = node->bounds.min;
    const glm::vec3 max    = node->bounds.max;

    // Dividimos en XZ, manteniendo Y
    // 0: bottom-left, 1: bottom-right, 2: top-left, 3: top-right (en XZ)
    AABB childBounds[4];

    // bottom-left
    childBounds[0] = AABB(
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(center.x, max.y, center.z)
    );

    // bottom-right
    childBounds[1] = AABB(
        glm::vec3(center.x, min.y, min.z),
        glm::vec3(max.x,    max.y, center.z)
    );

    // top-left
    childBounds[2] = AABB(
        glm::vec3(min.x,    min.y, center.z),
        glm::vec3(center.x, max.y, max.z)
    );

    // top-right
    childBounds[3] = AABB(
        glm::vec3(center.x, min.y, center.z),
        glm::vec3(max.x,    max.y, max.z)
    );

    for (int i = 0; i < 4; ++i)
    {
        node->children[i] = std::make_unique<Node>(childBounds[i]);
    }
}

int Quadtree::GetChildIndex(const Node* node, const AABB& box) const
{
    const glm::vec3 center = node->bounds.GetCenter();

    // Si la caja no cabe completamente en un solo hijo, devolvemos -1
    if (box.min.x <= center.x && box.max.x >= center.x) return -1;
    if (box.min.z <= center.z && box.max.z >= center.z) return -1;

    bool right = box.GetCenter().x > center.x;
    bool top   = box.GetCenter().z > center.z;

    // 0,1,2,3 como arriba
    if (!right && !top) return 0;
    if ( right && !top) return 1;
    if (!right &&  top) return 2;
    return 3; // right && top
}

void Quadtree::InsertRecursive(Node* node,
                               const std::shared_ptr<GameObject>& object,
                               const AABB& box,
                               int depth)
{
    // Si hemos llegado a la profundidad máxima, nos quedamos aquí
    if (depth >= mMaxDepth)
    {
        node->objects.push_back({object, box});
        return;
    }

    // Si no está subdividido y hay demasiados objetos, subdividir
    if (node->IsLeaf() && node->objects.size() >= static_cast<size_t>(mMaxObjectsPerNode))
    {
        Subdivide(node);

        // Recolocar los objetos antiguos en hijos si caben
        auto oldObjects = node->objects;
        node->objects.clear();

        for (const auto& entry : oldObjects)
        {
            const auto& obj  = entry.first;
            const auto& aabb = entry.second;

            int childIndex = GetChildIndex(node, aabb);
            if (childIndex != -1)
            {
                InsertRecursive(node->children[childIndex].get(), obj, aabb, depth + 1);
            }
            else
            {
                // No cabe completamente en un hijo, se queda en este nodo
                node->objects.push_back(entry);
            }
        }
    }

    if (!node->IsLeaf())
    {
        int childIndex = GetChildIndex(node, box);
        if (childIndex != -1)
        {
            InsertRecursive(node->children[childIndex].get(), object, box, depth + 1);
            return;
        }
    }

    // Si no cabe en ningún hijo o es hoja sin subdividir, se queda en este nodo
    node->objects.push_back({object, box});
}

// --- Raycast ---

bool Quadtree::RayIntersectsAABB(const Ray& ray, const AABB& box, float* tMinOut)
{
    glm::vec3 t0 = (box.min - ray.origin) / ray.direction;
    glm::vec3 t1 = (box.max - ray.origin) / ray.direction;

    glm::vec3 tMinVec = glm::min(t0, t1);
    glm::vec3 tMaxVec = glm::max(t0, t1);

    float tMin = std::max({ tMinVec.x, tMinVec.y, tMinVec.z });
    float tMax = std::min({ tMaxVec.x, tMaxVec.y, tMaxVec.z });

    bool hit = tMax >= std::max(tMin, 0.0f);
    if (hit && tMinOut) *tMinOut = tMin;
    return hit;
}

void Quadtree::QueryRay(const Ray& ray,
                        std::vector<std::shared_ptr<GameObject>>& outObjects) const
{
    if (!mRoot) return;
    outObjects.clear();
    QueryRayRecursive(mRoot.get(), ray, outObjects);
}

void Quadtree::QueryRayRecursive(const Node* node,
                                 const Ray& ray,
                                 std::vector<std::shared_ptr<GameObject>>& outObjects) const
{
    float t;
    if (!RayIntersectsAABB(ray, node->bounds, &t))
        return;

    // Añadir los objetos de este nodo
    for (const auto& entry : node->objects)
    {
        outObjects.push_back(entry.first);
    }

    // Recursión en hijos
    if (!node->IsLeaf())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (node->children[i])
                QueryRayRecursive(node->children[i].get(), ray, outObjects);
        }
    }
}

// --- Frustum ---

void Quadtree::QueryFrustum(const Frustum& frustum,
                            std::vector<std::shared_ptr<GameObject>>& outObjects) const
{
    if (!mRoot) return;
    outObjects.clear();
    QueryFrustumRecursive(mRoot.get(), frustum, outObjects);
}

void Quadtree::QueryFrustumRecursive(const Node* node,
                                     const Frustum& frustum,
                                     std::vector<std::shared_ptr<GameObject>>& outObjects) const
{
    if (!frustum.ContainsAABB(node->bounds))
        return;

    for (const auto& entry : node->objects)
    {
        outObjects.push_back(entry.first);
    }

    if (!node->IsLeaf())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (node->children[i])
                QueryFrustumRecursive(node->children[i].get(), frustum, outObjects);
        }
    }
}
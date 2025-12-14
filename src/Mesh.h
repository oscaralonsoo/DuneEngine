#pragma once
#include "Resource.h"
#include <glm/glm.hpp>
#include "AABB.h"
#include "MeshImportData.h"

class VertexArray;
class VertexBuffer;
class IndexBuffer;

struct Vertex
{
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Normals = glm::vec3(0.0f);
    glm::vec2 TexCoords = glm::vec2(0.0f);
    glm::vec3 Tangent = glm::vec3(0.0f);
    glm::vec3 Bitangent = glm::vec3(0.0f);
};

class Mesh : public Resource
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    ~Mesh() = default;

    const std::shared_ptr<VertexArray> &GetVertexArray() const;
    const std::shared_ptr<VertexBuffer> &GetVertexBuffer() const;
    const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const;

    void SetAABB(const AABB& aabb);
    const AABB& GetAABB() const;

    const std::vector<Vertex> &GetVertices() const;
    const std::vector<uint32_t> &GetIndices() const;

    void LoadFromImportData(const MeshImportData& data);

    // Override to calculate mesh memory size
    size_t GetMemorySize() const override;

private:
    std::shared_ptr<VertexArray> mVertexArray;
    std::shared_ptr<VertexBuffer> mVertexBuffer;
    std::shared_ptr<IndexBuffer> mIndexBuffer;

    std::vector<uint32_t> mIndices;
    std::vector<Vertex> mVertices;
    AABB mAABB;
};
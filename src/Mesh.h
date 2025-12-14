#pragma once

#include "Resource.h"
#include "MeshImportData.h"
#include "AABB.h"
#include "Vertex.h"

#include <glm/glm.hpp>

class VertexArray;
class VertexBuffer;
class IndexBuffer;

class Mesh : public Resource
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    Mesh(const MeshImportData &importData);
    ~Mesh() = default;

    const std::shared_ptr<VertexArray> &GetVertexArray() const;
    const std::shared_ptr<VertexBuffer> &GetVertexBuffer() const;
    const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const;

    void SetAABB(const AABB &aabb);
    const AABB &GetAABB() const;

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
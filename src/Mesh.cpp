#include "Mesh.h"
#include "VertexArray.h"
#include "Buffer.h"

static AABB ComputeLocalAABB(const std::vector<Vertex>& vertices)
{
    AABB box;
    for (const auto& v : vertices)
    {
        box.Encapsulate(v.Position);
    }
    return box;
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    : Resource(ResourceType::Mesh), mVertices(vertices), mIndices(indices)
{
    mAABB = ComputeLocalAABB(mVertices);

    mVertexBuffer = std::make_shared<VertexBuffer>((float *)mVertices.data(), mVertices.size() * sizeof(Vertex));
    mIndexBuffer = std::make_shared<IndexBuffer>(mIndices.data(), mIndices.size());

    BufferLayout layout = {
        {ShaderDataType::Vec3, "aPosition"},
        {ShaderDataType::Vec3, "aNormals"},
        {ShaderDataType::Vec2, "aTexCoords"},
        {ShaderDataType::Vec3, "aTangent"},
        {ShaderDataType::Vec3, "aBitangent"}};

    mVertexBuffer->SetLayout(layout);

    mVertexArray = std::make_shared<VertexArray>();
    mVertexArray->AddVertexBuffer(mVertexBuffer);
    mVertexArray->SetIndexBuffer(mIndexBuffer);
}

const std::shared_ptr<VertexArray> &Mesh::GetVertexArray() const
{
    return mVertexArray;
}

const std::shared_ptr<VertexBuffer> &Mesh::GetVertexBuffer() const
{
    return mVertexBuffer;
}

const std::shared_ptr<IndexBuffer> &Mesh::GetIndexBuffer() const
{
    return mIndexBuffer;
}

void Mesh::SetAABB(const AABB &aabb)
{
    mAABB = aabb;
}

const AABB &Mesh::GetAABB() const
{
    return mAABB;
}

const std::vector<uint32_t> &Mesh::GetIndices() const
{
    return mIndices;
}

const std::vector<Vertex> &Mesh::GetVertices() const
{
    return mVertices;
}
#include "Mesh.h"
#include "VertexArray.h"
#include "Buffer.h"

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    : Resource(ResourceType::Mesh), mVertices(vertices), mIndices(indices)
{
    mVertexBuffer = std::make_shared<VertexBuffer>((float *)mVertices.data(), mVertices.size() * sizeof(Vertex));
    mIndexBuffer = std::make_shared<IndexBuffer>(mIndices.data(), mIndices.size());

    BufferLayout layout = {
        {ShaderDataType::Vec3, "aPosition"},
        {ShaderDataType::Vec3, "aNormals"},
        {ShaderDataType::Vec2, "aTexCoords"},
        {ShaderDataType::Vec3, "aTangent"}
    };

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

const std::vector<uint32_t> &Mesh::GetIndices() const
{
    return mIndices;
}

const std::vector<Vertex> &Mesh::GetVertices() const
{
    return mVertices;
}
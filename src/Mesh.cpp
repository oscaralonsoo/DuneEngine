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

void Mesh::LoadFromImportData(const MeshImportData& data)
{
    mVertices.clear();
    mIndices = data.indices;

    // Layout esperado:
    // pos(3), normal(3), uv(2), tangent(3), bitangent(3) = 14 floats
    constexpr size_t stride = 14;

    for (size_t i = 0; i < data.vertices.size(); i += stride)
    {
        Vertex v;

        v.Position = {
            data.vertices[i + 0],
            data.vertices[i + 1],
            data.vertices[i + 2]
        };

        v.Normals = {
            data.vertices[i + 3],
            data.vertices[i + 4],
            data.vertices[i + 5]
        };

        v.TexCoords = {
            data.vertices[i + 6],
            data.vertices[i + 7]
        };

        v.Tangent = {
            data.vertices[i + 8],
            data.vertices[i + 9],
            data.vertices[i + 10]
        };

        v.Bitangent = {
            data.vertices[i + 11],
            data.vertices[i + 12],
            data.vertices[i + 13]
        };

        mVertices.push_back(v);
    }

    // reconstruir buffers usando el constructor existente
    mVertexArray.reset();
    mVertexBuffer.reset();
    mIndexBuffer.reset();

    *this = Mesh(mVertices, mIndices);
}
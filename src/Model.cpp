#include "Model.h"
#include "ModelImportData.h"
#include "Globals.h"

Model::Model(const ModelImportData &importData)
    : Resource(ResourceType::Model)
{
    for (const auto &meshData : importData.meshes)
    {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(meshData);
        mMeshes.push_back(mesh);
    }
}

void Model::ProcessNodeHierarchy(aiNode *node, const aiScene *scene)
{
    mNodeName = node->mName.C_Str();

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        AddMesh(ProcessMesh(mesh, scene));
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNodeHierarchy(node->mChildren[i], scene);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;

        vertex.Position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;

            vertex.Normals = vector;
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    AABB aabb(
        glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z),
        glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z));

    auto meshPtr = std::make_shared<Mesh>(vertices, indices);
    meshPtr->SetName(mesh->mName.C_Str());
    meshPtr->SetAABB(aabb);

    // TODO: Add process materials

    return meshPtr;
}

size_t Model::GetMemorySize() const
{
    size_t totalSize = sizeof(Model);
    
    // Add memory from all meshes
    for (const auto& mesh : mMeshes)
    {
        if (mesh)
            totalSize += mesh->GetMemorySize();
    }
    
    // Add memory from children models
    for (const auto& child : mChildren)
    {
        if (child)
            totalSize += child->GetMemorySize();
    }
    
    return totalSize;
}

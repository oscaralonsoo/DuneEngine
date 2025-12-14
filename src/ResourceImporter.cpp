#include "ResourceImporter.h"
#include "Vertex.h"

#include "IL/il.h"
#include <fstream>

TextureImportData ResourceImporter::Import(const std::filesystem::path &path, TextureImportData importData)
{
    std::ifstream f(path, std::ios::binary);

    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());

    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);

    if (!ilLoadL(IL_TYPE_UNKNOWN, buf.data(), static_cast<ILuint>(buf.size())))
    {
        ILenum err = ilGetError();
        ilDeleteImages(1, &img);
    }

    importData.width = ilGetInteger(IL_IMAGE_WIDTH);
    importData.height = ilGetInteger(IL_IMAGE_HEIGHT);
    importData.channels = ilGetInteger(IL_IMAGE_CHANNELS);

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    importData.channels = 4;

    size_t dataSize = importData.width * importData.height * importData.channels;
    importData.data.resize(dataSize);
    memcpy(importData.data.data(), ilGetData(), dataSize);

    ilDeleteImages(1, &img);

    return importData;
}

ModelImportData ResourceImporter::Import(const std::filesystem::path &path, ModelImportData importData)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_GenBoundingBoxes
    );

    ProcessModelNodes(scene->mRootNode, scene, importData);

    return importData;
}

void ResourceImporter::ProcessModelNodes(aiNode* node, const aiScene* scene, ModelImportData &modelData)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshImportData meshData = ProcessMesh(mesh, scene);
        modelData.meshes.push_back(meshData);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        ProcessModelNodes(node->mChildren[i], scene, modelData);
    }
}

MeshImportData ResourceImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    MeshImportData meshData;

    meshData.vertices.resize(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
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
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

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

        meshData.vertices[i] = vertex;
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
            meshData.indices.push_back(face.mIndices[j]);
    }

    // TODO: process materials if needed

    return meshData;
}
#pragma once

#include "Texture.h"
#include "TextureImportData.h"
#include "MeshImportData.h"
#include "ModelImportData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ResourceImporter
{
public:
    ResourceImporter() = default;
    ~ResourceImporter() = default;

    static TextureImportData Import(const std::filesystem::path &path, TextureImportData importData);
    static ModelImportData Import(const std::filesystem::path &path, ModelImportData importData);

private:
    static void ProcessModelNodes(aiNode *node, const aiScene *scene, ModelImportData &modelData);
    static MeshImportData ProcessMesh(aiMesh *mesh, const aiScene *scene);
};

#pragma once

#include "Resource.h"
#include "ModelImportData.h"
#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

class Model : public Resource, public std::enable_shared_from_this<Model>
{
public:
    Model(const ModelImportData &importData);

    ~Model() = default;

    const std::vector<std::shared_ptr<Mesh>> &GetMeshes() const { return mMeshes; };

    void AddMesh(const std::shared_ptr<Mesh> mesh) { mMeshes.push_back(mesh); };

    const std::string &GetNodeName() { return mNodeName; };

    const std::weak_ptr<Model> GetParent() const { return mParent; }
    const std::vector<std::shared_ptr<Model>> GetChildren() const { return mChildren; }

    const glm::mat4 GetTransform() const { return mTransform; }

private:
    void ProcessNodeHierarchy(aiNode *node, const aiScene *scene);
    std::shared_ptr<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene);

private:
    std::filesystem::path mFilePath;
    std::vector<std::shared_ptr<Mesh>> mMeshes;
    std::weak_ptr<Model> mParent;
    std::vector<std::shared_ptr<Model>> mChildren;
    std::string mNodeName;
    glm::mat4 mTransform;
};

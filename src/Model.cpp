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
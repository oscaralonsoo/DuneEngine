#include "Model.h"
#include "AABB.h"
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

size_t Model::GetMemorySize() const
{
    size_t totalSize = sizeof(Model);

    // Add memory from all meshes
    for (const auto &mesh : mMeshes)
    {
        if (mesh)
            totalSize += mesh->GetMemorySize();
    }

    // Add memory from children models
    for (const auto &child : mChildren)
    {
        if (child)
            totalSize += child->GetMemorySize();
    }

    return totalSize;
}

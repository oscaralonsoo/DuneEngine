#include "ModuleResource.h"
#include "ResourceImporter.h"
#include "ResourceSaver.h"
#include "ResourceLoader.h"
#include "Texture.h"
#include "ResourceUtils.h"
#include "Globals.h"

std::shared_ptr<Resource> ModuleResource::RequestResource(std::filesystem::path assetPath)
{
    std::filesystem::path metaPath = assetPath;
    metaPath += ".meta";

    if (!std::filesystem::exists(metaPath))
    {
        return Find(ImportFile(assetPath));
    }

    UID uid = ResourceUtils::GetUIDFromMeta(metaPath);

    if (auto resource = Find(uid))
        return resource;

 
    return CreateResource(ResourceLoader::Load(uid));
}

UID ModuleResource::ImportFile(std::filesystem::path assetPath)
{
    TextureImportData importData;

    importData.uid = UID();
    importData.assetPath = assetPath;

    ResourceType type = ResourceUtils::GetTypeFromExtension(assetPath);

    switch (type)
    {
    case ResourceType::Texture:
        importData = ResourceImporter::Import(assetPath, importData);
        break;

    default:
        break;
    }

    ResourceSaver::Save(importData);

    CreateResource(importData);

    return importData.uid;
}

std::shared_ptr<Resource> ModuleResource::Find(UID uid)
{
    auto it = mResources.find(uid.ToString());
    if (it != mResources.end())
    {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<Resource> ModuleResource::CreateResource(TextureImportData importData)
{
    std::shared_ptr<Resource> resource = nullptr;

    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        resource = std::make_shared<Texture>(importData);
        break;
    }
    case ResourceType::Mesh:
    {
        // resource = std::make_shared<Mesh>(importData);
        break;
    }
    case ResourceType::Material:
    {
        // resource = std::make_shared<Material>(importData);
        break;
    }
    default:
        break;
    }

    if (resource)
    {
        mResources[importData.uid.ToString()] = resource;
    }

    return resource;
}

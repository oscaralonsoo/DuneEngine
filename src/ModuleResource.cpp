#include "ModuleResource.h"
#include "ResourceImporter.h"
#include "ResourceSaver.h"
#include "ResourceLoader.h"
#include "Texture.h"
#include "ResourceUtils.h"
#include "Globals.h"

#include "IL/il.h"

bool ModuleResource::Awake()
{
    // Devil Init
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    return true;
}

bool ModuleResource::Start()
{

    return true;
}

std::shared_ptr<Resource> ModuleResource::RequestResource(std::filesystem::path assetPath)
{
    std::filesystem::path metaPath = assetPath;
    metaPath += ".meta";

    if (!std::filesystem::exists(metaPath))
        return Find(ImportFile(assetPath));

    UID uid = ResourceUtils::GetUIDFromMeta(metaPath);

    if (auto resource = Find(uid))
        return resource;

    std::string binPath = ResourceUtils::GetLibraryPath(uid);
    if (!binPath.empty())
    {
        return CreateResource(ResourceLoader::Load(uid));
    }
    else
    {
        LOG_INFO("adios");
        return Find(ImportFile(assetPath));
    }
}

UID ModuleResource::ImportFile(std::filesystem::path assetPath)
{
    UID uid = UID();

    switch (ResourceUtils::GetTypeFromExtension(assetPath))
    {
    case ResourceType::Texture:
    {
        TextureImportData importData;
        importData.assetPath = assetPath;
        importData.uid = uid;
        importData.type = ResourceType::Texture;

        importData = ResourceImporter::Import(assetPath, importData);

        ResourceSaver::Save(importData);

        CreateResource(importData);
        break;
    }

        // case ResourceType::Mesh:
        //     importData = std::make_unique<MeshImportData>();
        //     ResourceImporter::Import(assetPath, static_cast<MeshImportData&>(*importData));
        //     break;
    }

    return uid;
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

std::shared_ptr<Resource> ModuleResource::CreateResource(const ImportData &importData)
{
    std::shared_ptr<Resource> resource = nullptr;

    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        resource = std::make_shared<Texture>(static_cast<const TextureImportData &>(importData));
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

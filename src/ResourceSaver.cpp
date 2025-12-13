#include "ResourceSaver.h"
#include "ResourceUtils.h"
#include "Globals.h"

#include <fstream>

void ResourceSaver::Save(const ImportData& importData)
{
    SaveBinary(importData);

    SaveMeta(importData);
}


void ResourceSaver::SaveMeta(const ImportData& importData)
{
    std::filesystem::path metaPath = importData.assetPath;
    metaPath += ".meta";

    std::ofstream file(metaPath);
    if (!file.is_open())
        return;

    file << "<Meta uid=\"" << importData.uid.ToString()
         << "\" type=\"" << ResourceUtils::ToString(importData.type) << "\"/>\n";
}

void ResourceSaver::SaveBinary(const ImportData& importData)
{
    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        const TextureImportData& textureData = static_cast<const TextureImportData&>(importData);
        std::string outputPath = "Library/Textures/" + textureData.uid.ToString() + ".bin";
        std::ofstream out(outputPath, std::ios::binary);
        out << textureData;
        break;
    }
    // case ResourceType::Mesh:
    //     const MeshImportData& meshData = static_cast<const MeshImportData&>(importData);
    //     SaveMeshBinary(meshData);
    //     break;
    default:
        SDL_Log("SaveBinary: tipo de recurso no soportado: %d", static_cast<int>(importData.type));
        break;
    }
}


std::ostream &operator<<(std::ostream &os, const TextureImportData &texture)
{
    os.write((const char *)&texture.width, sizeof(texture.width));
    os.write((const char *)&texture.height, sizeof(texture.height));
    os.write((const char *)&texture.channels, sizeof(texture.channels));
    os.write(texture.data.data(), texture.data.size());

    return os;
}
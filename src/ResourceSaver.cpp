#include "ResourceSaver.h"
#include "ResourceUtils.h"
#include "Globals.h"
#include "MeshImportData.h"

#include <fstream>

void ResourceSaver::Save(const ImportData& importData)
{
    SaveBinary(importData);

    SaveMeta(importData);
}

bool ResourceSaver::SaveMesh(const MeshImportData& data, const std::string& path)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    file << data;
    return true;
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
    case ResourceType::Mesh:
    {
        const MeshImportData& meshData =
            static_cast<const MeshImportData&>(importData);

        std::string outputPath =
            "Library/Meshes/" + meshData.uid.ToString() + ".bin";

        std::ofstream out(outputPath, std::ios::binary);
        out << meshData;
        break;
    }
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

std::ostream& operator<<(std::ostream& os, const MeshImportData& data)
{
    uint32_t vertexCount = static_cast<uint32_t>(data.vertices.size());
    uint32_t indexCount  = static_cast<uint32_t>(data.indices.size());

    os.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
    os.write(reinterpret_cast<const char*>(data.vertices.data()),
             sizeof(float) * vertexCount);

    os.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
    os.write(reinterpret_cast<const char*>(data.indices.data()),
             sizeof(uint32_t) * indexCount);

    return os;
}
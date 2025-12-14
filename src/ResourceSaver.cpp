#include "ResourceSaver.h"
#include "ResourceUtils.h"
#include "Globals.h"
#include "MeshImportData.h"

#include <fstream>

void ResourceSaver::Save(const ImportData &importData)
{
    SaveBinary(importData);

    SaveMeta(importData);
}
void ResourceSaver::SaveMeta(const ImportData &importData)
{
    std::filesystem::path metaPath = importData.assetPath;
    metaPath += ".meta";

    std::ofstream file(metaPath);
    if (!file.is_open())
        return;

    file << "<Meta uid=\"" << importData.uid.ToString()
         << "\" type=\"" << ResourceUtils::ToString(importData.type) << "\"/>\n";
}

void ResourceSaver::SaveBinary(const ImportData &importData)
{
    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        const TextureImportData &textureData = static_cast<const TextureImportData &>(importData);
        std::string outputPath = "Library/" + textureData.uid.ToString() + ".tex";
        std::ofstream out(outputPath, std::ios::binary);
        out << textureData;
        break;
    }
    case ResourceType::Model:
    {
        const ModelImportData &modelData =
            static_cast<const ModelImportData &>(importData);

        std::string outputPath =
            "Library/" + modelData.uid.ToString() + ".model";

        std::ofstream out(outputPath, std::ios::binary);
        out << modelData;
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

std::ostream &operator<<(std::ostream &os, const MeshImportData &data)
{
    uint32_t vertexCount = static_cast<uint32_t>(data.vertices.size());
    os.write(reinterpret_cast<const char *>(&vertexCount), sizeof(uint32_t));

    for (const auto &v : data.vertices)
    {
        os.write(reinterpret_cast<const char *>(&v.Position), sizeof(glm::vec3));
        os.write(reinterpret_cast<const char *>(&v.Normals), sizeof(glm::vec3));
        os.write(reinterpret_cast<const char *>(&v.TexCoords), sizeof(glm::vec2));
        os.write(reinterpret_cast<const char *>(&v.Tangent), sizeof(glm::vec3));
        os.write(reinterpret_cast<const char *>(&v.Bitangent), sizeof(glm::vec3));
    }

    uint32_t indexCount = static_cast<uint32_t>(data.indices.size());
    os.write(reinterpret_cast<const char *>(&indexCount), sizeof(uint32_t));
    os.write(reinterpret_cast<const char *>(data.indices.data()), sizeof(uint32_t) * indexCount);

    os.write(reinterpret_cast<const char *>(&data.aabb.min), sizeof(glm::vec3));
    os.write(reinterpret_cast<const char *>(&data.aabb.max), sizeof(glm::vec3));

    return os;
}

std::ostream &operator<<(std::ostream &os, const ModelImportData &data)
{
    uint32_t meshCount = static_cast<uint32_t>(data.meshes.size());
    SDL_Log("Saving Model: meshes=%u", meshCount);

    os.write(reinterpret_cast<const char *>(&meshCount), sizeof(uint32_t));

    for (const auto &mesh : data.meshes)
    {
        os << mesh;
    }

    return os;
}
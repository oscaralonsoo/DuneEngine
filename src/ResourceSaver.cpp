#include "ResourceSaver.h"
#include "ResourceUtils.h"
#include "Globals.h"

#include <fstream>

void ResourceSaver::Save(const TextureImportData &importData)
{
    SaveBinary(importData);

    SaveMeta(importData);
}

void ResourceSaver::SaveMeta(const TextureImportData &importData)
{
    std::filesystem::path metaPath = importData.assetPath;
    metaPath += ".meta";
    
    std::ofstream file(metaPath);
    if (!file.is_open())
        return;

    file << "<Meta uid=\"" << importData.uid.ToString()
         << "\" type=\"" << ResourceUtils::ToString(importData.type) << "\"/>\n";
    file.close();
}

void ResourceSaver::SaveBinary(const TextureImportData &importData)
{
    switch (importData.type)
    {
    case ResourceType::Texture:
    {
        std::string outputPath = "Assets/Textures/" + importData.uid.ToString() + ".texbin";

        std::ofstream out(outputPath, std::ios::binary);

        out << importData;

        out.close();
        break;
    }
    default:
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
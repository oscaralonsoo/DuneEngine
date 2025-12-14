#include "ResourceLoader.h"
#include "ResourceUtils.h"
#include "Globals.h"

#include <fstream>

TextureImportData ResourceLoader::LoadTexture(const UID uid)
{
    std::string libraryPath = ResourceUtils::GetLibraryPath(uid);
    std::ifstream in(libraryPath, std::ios::binary);
    TextureImportData importData;
    importData.uid = uid;
    in >> importData;
    return importData;
}

ModelImportData ResourceLoader::LoadModel(const UID uid)
{
    std::string libraryPath = ResourceUtils::GetLibraryPath(uid);
    std::ifstream in(libraryPath, std::ios::binary);
    ModelImportData importData;
    importData.uid = uid;
    in >> importData;
    return importData;
}

std::istream &operator>>(std::istream &is, TextureImportData &texture)
{
    is.read((char *)&texture.width, sizeof(texture.width));
    is.read((char *)&texture.height, sizeof(texture.height));
    is.read((char *)&texture.channels, sizeof(texture.channels));
    texture.data.resize(texture.width * texture.height * texture.channels);
    is.read(texture.data.data(), texture.data.size());
    return is;
}

std::istream &operator>>(std::istream &is, ModelImportData &data)
{
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    is.read(reinterpret_cast<char *>(&vertexCount), sizeof(uint32_t));
    // data.vertices.resize(vertexCount);
    // is.read(reinterpret_cast<char *>(data.vertices.data()),
    //         sizeof(float) * vertexCount);

    // is.read(reinterpret_cast<char *>(&indexCount), sizeof(uint32_t));
    // data.indices.resize(indexCount);
    // is.read(reinterpret_cast<char *>(data.indices.data()),
    //         sizeof(uint32_t) * indexCount);

    return is;
}
#include "ResourceLoader.h"
#include "ResourceUtils.h"
#include "Globals.h"

#include <fstream>

TextureImportData ResourceLoader::Load(const UID uid)
{
    std::ifstream in(ResourceUtils::GetLibraryPath(uid), std::ios::binary);

    TextureImportData importData;
    importData.uid = uid;
    in >> importData;

    in.close();

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
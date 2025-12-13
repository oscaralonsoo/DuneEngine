#include "ResourceImporter.h"
#include "IL/il.h"

#include <fstream>

TextureImportData ResourceImporter::Import(const std::filesystem::path &path, TextureImportData importData)
{
    static bool initialized = false;
    if (!initialized)
    {
        ilInit();
        initialized = true;
    }

    std::ifstream f(path, std::ios::binary);

    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());

    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);

    if (!ilLoadL(IL_TYPE_UNKNOWN, buf.data(), static_cast<ILuint>(buf.size())))
    {
        ILenum err = ilGetError();
        ilDeleteImages(1, &img);
    }

    importData.width = ilGetInteger(IL_IMAGE_WIDTH);
    importData.height = ilGetInteger(IL_IMAGE_HEIGHT);
    importData.channels = ilGetInteger(IL_IMAGE_CHANNELS);

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    importData.channels = 4;

    size_t dataSize = importData.width * importData.height * importData.channels;
    importData.data.resize(dataSize);
    memcpy(importData.data.data(), ilGetData(), dataSize);

    ilDeleteImages(1, &img);

    return importData;
}
#pragma once

#include "ImportData.h"

struct TextureImportData : public ImportData
{
    int width = 0;
    int height = 0;
    int channels = 0;

    std::vector<char> data;

    TextureImportData() : ImportData(ResourceType::Texture) {}
};
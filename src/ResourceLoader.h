#pragma once

#include "TextureImportData.h"

class ResourceLoader
{
public:
    ResourceLoader() = default;
    ~ResourceLoader() = default;

    static TextureImportData Load(const UID uid);
    static void Unload(const ImportData data);
};

std::istream &operator>>(std::istream &is, TextureImportData &texture);
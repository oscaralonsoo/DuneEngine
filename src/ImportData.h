#pragma once

#include "Resource.h"

struct ImportData
{
    UID uid;
    ResourceType type = ResourceType::Unknown;

    std::filesystem::path assetPath;
    std::filesystem::path libraryPath;

    ImportData() = default;
    ImportData(ResourceType type) : type(type) {}
    virtual ~ImportData() = default;
};
#pragma once

#include "Resource.h"

struct ImportData
{
    UID uid;
    ResourceType type = ResourceType::Unknown;

    std::filesystem::path assetPath;
    std::filesystem::path libraryPath;

    //uint64_t sourceTimestamp = 0;
    //uint64_t sourceHash = 0;

    ImportData() = default;
    ImportData(ResourceType type) : type(type) {}
    virtual ~ImportData() = default;
};
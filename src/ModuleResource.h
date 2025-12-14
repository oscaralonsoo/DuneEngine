#pragma once

#include "Module.h"
#include "Resource.h"
#include "ImportData.h"
#include "UID.h"

#include <map>

class ModuleResource : public Module
{
public:
    bool Awake();
    bool Start();

    UID ImportFile(std::filesystem::path filePath);
    std::shared_ptr<Resource> Find(UID uid);
    std::shared_ptr<Resource> RequestResource(std::filesystem::path);

    // Get the number of GameObjects using a specific resource
    int GetResourceUsageCount(UID resourceUID) const;

private:
    std::shared_ptr<Resource> CreateResource(const ImportData &data);

private:
    std::map<std::string, std::shared_ptr<Resource>> mResources;
};
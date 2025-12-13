#pragma once

#include "Module.h"
#include "Resource.h"
#include "UID.h"
#include "TextureImportData.h"

#include <map>

class ModuleResource : public Module
{
public:
    UID ImportFile(std::filesystem::path filePath);
    std::shared_ptr<Resource> Find(UID uid);
    std::shared_ptr<Resource> RequestResource(std::filesystem::path);

private:
    std::shared_ptr<Resource> CreateResource(TextureImportData data);

private:
    std::map<std::string, std::shared_ptr<Resource>> mResources;
};
#pragma once

#include "UID.h"
#include "ImportData.h"
#include "MeshImportData.h"

struct ModelImportData : public ImportData
{
    std::vector<MeshImportData> meshes;

    ModelImportData() : ImportData(ResourceType::Model) {}
};
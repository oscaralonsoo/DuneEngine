#pragma once

#include "ImportData.h"

struct ModelImportData : ImportData
{
    int id;
    const char* name;
    double value;
};
#include "FileDialogUtils.h"
#include <string>
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>

std::string FileDialogUtils::OpenImageFile()
{
    // Save current directory
    CHAR currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    OPENFILENAMEA openFileName;
    CHAR szFile[260] = { 0 };
    CHAR szFilter[] = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.dds\0All Files\0*.*\0";

    ZeroMemory(&openFileName, sizeof(openFileName));
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = nullptr;
    openFileName.lpstrFile = szFile;
    openFileName.nMaxFile = sizeof(szFile);
    openFileName.lpstrFilter = szFilter;
    openFileName.nFilterIndex = 1;
    openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    std::string result;
    if (GetOpenFileNameA(&openFileName) == TRUE)
        result = std::string(openFileName.lpstrFile);

    // Restore current directory
    SetCurrentDirectoryA(currentDir);

    return result;
}

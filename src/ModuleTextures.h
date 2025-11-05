#pragma once

#include "Module.h"
#include <string>
#include <unordered_map>
#include <IL/il.h>
#include <glad/glad.h>

class ModuleTextures : public Module
{
public:
    ModuleTextures();
    ~ModuleTextures();

    bool Awake() override;
    bool CleanUp() override;

    GLuint LoadTexture(const std::string& path, bool genMipmaps = true);
    void UnloadTexture(GLuint texture);
    void UnloadAll();

private:
    std::unordered_map<std::string, GLuint> textures;
};

#include "ModuleTextures.h"
#include <fstream>
#include <vector>

ModuleTextures::ModuleTextures() { name = "textures"; }
ModuleTextures::~ModuleTextures() {}

bool ModuleTextures::Awake()
{
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    return true;
}

bool ModuleTextures::CleanUp()
{
    UnloadAll();
    return true;
}

GLuint ModuleTextures::LoadTexture(const std::string& path, bool genMipmaps)
{
    if (textures.find(path) != textures.end())
        return textures[path];

    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        //SDL_Log("No se pudo abrir %s", path.c_str());
        return 0;
    }
    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());

    ILuint img = 0;
    ilGenImages(1, &img);
    ilBindImage(img);

    if (!ilLoadL(IL_TYPE_UNKNOWN, buf.data(), (ILuint)buf.size()))
    {
        ILenum err = ilGetError();
        //SDL_Log("DevIL: fallo al cargar '%s' (err=%d)", path.c_str(), (int)err);
        ilDeleteImages(1, &img);
        return 0;
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    const int w = ilGetInteger(IL_IMAGE_WIDTH);
    const int h = ilGetInteger(IL_IMAGE_HEIGHT);
    const void* pixels = ilGetData();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    if (genMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    ilDeleteImages(1, &img);
    textures[path] = tex;
    return tex;
}

void ModuleTextures::UnloadTexture(GLuint texture)
{
    if (texture != 0)
        glDeleteTextures(1, &texture);
}

void ModuleTextures::UnloadAll()
{
    for (auto& t : textures)
        glDeleteTextures(1, &t.second);
    textures.clear();
}

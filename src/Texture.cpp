#include "Texture.h"
#include "Globals.h"

#include <glad/glad.h>
#include <cstdint>
#include <IL/il.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

GLenum ToGLWrap(TextureWrap wrap)
{
    switch (wrap)
    {
    case TextureWrap::Repeat:
        return GL_REPEAT;
    case TextureWrap::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    case TextureWrap::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case TextureWrap::ClampToBorder:
        return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

GLenum ToGLFilter(TextureFilter filter)
{
    switch (filter)
    {
    case TextureFilter::Nearest:
        return GL_NEAREST;
    case TextureFilter::Linear:
        return GL_LINEAR;
    case TextureFilter::NearestMipmapNearest:
        return GL_NEAREST_MIPMAP_NEAREST;
    case TextureFilter::LinearMipmapNearest:
        return GL_LINEAR_MIPMAP_NEAREST;
    case TextureFilter::NearestMipmapLinear:
        return GL_NEAREST_MIPMAP_LINEAR;
    case TextureFilter::LinearMipmapLinear:
        return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

GLenum ToGLInternalFormat(ImageFormat format, bool srgb)
{
    switch (format)
    {
    case ImageFormat::R8:
        return GL_R8;
    case ImageFormat::RG8:
        return GL_RG8;
    case ImageFormat::RGB8:
        return srgb ? GL_SRGB8 : GL_RGB8;
    case ImageFormat::SRGB8:
        return GL_SRGB8;
    case ImageFormat::RGBA8:
        return srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    case ImageFormat::SRGBA8:
        return GL_SRGB8_ALPHA8;
    case ImageFormat::R16F:
        return GL_R16F;
    case ImageFormat::RG16F:
        return GL_RG16F;
    case ImageFormat::RGB16F:
        return GL_RGB16F;
    case ImageFormat::RGBA16F:
        return GL_RGBA16F;
    case ImageFormat::R32F:
        return GL_R32F;
    case ImageFormat::RGB32F:
        return GL_RGB32F;
    case ImageFormat::RGBA32F:
        return GL_RGBA32F;
    case ImageFormat::DEPTH24STENCIL8:
        return GL_DEPTH24_STENCIL8;
    }
    return GL_RGBA8;
}

GLenum ToGLFormat(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::R8:
    case ImageFormat::R16F:
    case ImageFormat::R32F:
        return GL_RED;
    case ImageFormat::RG8:
    case ImageFormat::RG16F:
        return GL_RG;
    case ImageFormat::RGB8:
    case ImageFormat::SRGB8:
    case ImageFormat::RGB16F:
    case ImageFormat::RGB32F:
        return GL_RGB;
    case ImageFormat::RGBA8:
    case ImageFormat::SRGBA8:
    case ImageFormat::RGBA16F:
    case ImageFormat::RGBA32F:
        return GL_RGBA;
    case ImageFormat::DEPTH24STENCIL8:
        return GL_DEPTH_STENCIL;
    }
    return GL_RGBA;
}

Texture::Texture(const std::filesystem::path &path, bool srgb)
{
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    
    mProperties.srgb = srgb;
    mID = LoadFromFile(path);
}

Texture::~Texture()
{
    if (mID != 0)
        glDeleteTextures(1, &mID);
}

void Texture::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, mID);
}

void Texture::Resize(GLuint width, GLuint height)
{
    mProperties.width = width;
    mProperties.height = height;

    if (mID != 0)
        glDeleteTextures(1, &mID);

    mID = InitializeTexture(nullptr, width, height); // Reservar memoria vacía
}

void Texture::Clear(const glm::vec4 &color)
{
    Bind();
    glClearTexImage(mID, 0, ToGLFormat(mProperties.format), GL_FLOAT, &color);
}

GLuint Texture::LoadFromFile(const std::filesystem::path &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return 0;
    }

    std::vector<unsigned char> buf((std::istreambuf_iterator<char>(f)),
                                   std::istreambuf_iterator<char>());

    ILuint img = 0;
    ilGenImages(1, &img);
    ilBindImage(img);

    if (!ilLoadL(IL_TYPE_UNKNOWN, buf.data(), static_cast<ILuint>(buf.size())))
    {
        ILenum err = ilGetError();
        ilDeleteImages(1, &img);
        return 0;
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    mProperties.width = ilGetInteger(IL_IMAGE_WIDTH);
    mProperties.height = ilGetInteger(IL_IMAGE_HEIGHT);
    const void *pixels = ilGetData();

    GLuint tex = InitializeTexture(pixels, mProperties.width, mProperties.height);

    ilDeleteImages(1, &img);
    return tex;
}

GLuint Texture::InitializeTexture(const void *pixels, GLuint width, GLuint height)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);

    mID = tex;
    Bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLWrap(mProperties.wrapping));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLWrap(mProperties.wrapping));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLFilter(mProperties.minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLFilter(mProperties.magFilter));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum internalFormat = ToGLInternalFormat(mProperties.format, mProperties.srgb);
    GLenum dataFormat = ToGLFormat(mProperties.format);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, pixels);

    if (mProperties.genMipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return tex;
}
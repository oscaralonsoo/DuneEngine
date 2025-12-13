#include "Texture.h"
#include "Globals.h"
#include "ResourceLoader.h"

#include <glad/glad.h>
#include <cstdint>
#include <filesystem>
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

Texture::Texture(const TextureProperties &props, const void *pixelData) : Resource(ResourceType::Texture)
{
    mProperties = props;
    mID = InitializeTexture(pixelData, props.width, props.height);
}

Texture::Texture(const TextureImportData importData) : Resource(ResourceType::Texture)
{
    mProperties.width = static_cast<GLuint>(importData.width);
    mProperties.height = static_cast<GLuint>(importData.height);

    switch (importData.channels)
    {
    case 1:
        mProperties.format = ImageFormat::R8;
        break;
    case 2:
        mProperties.format = ImageFormat::RG8;
        break;
    case 3:
        mProperties.format = ImageFormat::RGB8;
        break;
    case 4:
        mProperties.format = ImageFormat::RGBA8;
        break;
    default:
        mProperties.format = ImageFormat::RGBA8;
        break;
    }

    mData = new unsigned char[importData.width * importData.height * importData.channels];
    std::memcpy(mData, importData.data.data(), importData.width * importData.height * importData.channels);

    mID = InitializeTexture(mData, mProperties.width, mProperties.height);
}

Texture::~Texture()
{
    if (mID != 0)
        glDeleteTextures(1, &mID);
}

void Texture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, mID);
}

void Texture::Clear(const glm::vec4 &color)
{
    Bind(0);
    glClearTexImage(mID, 0, ToGLFormat(mProperties.format), GL_FLOAT, &color);
}

GLuint Texture::InitializeTexture(const void *pixels, GLuint width, GLuint height)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);

    mID = tex;
    Bind(0);

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

GLenum Texture::GetImageFormat() const
{
    return ToGLFormat(mProperties.format);
}

GLenum Texture::GetImageInternalFormat() const
{
    return ToGLInternalFormat(mProperties.format, mProperties.srgb);
}
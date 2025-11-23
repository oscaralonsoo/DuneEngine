#pragma once

#include "Resource.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <filesystem>
#include <vector>

enum class ImageFormat { R8, RG8, RGB8, SRGB8, RGBA8, SRGBA8, R16F, RG16F, RGB16F, RGBA16F, R32F, RGB32F, RGBA32F, DEPTH24STENCIL8 };
enum class TextureWrap { Repeat, MirroredRepeat, ClampToEdge, ClampToBorder };
enum class TextureFilter { Nearest, Linear, NearestMipmapNearest, LinearMipmapNearest, NearestMipmapLinear, LinearMipmapLinear };

struct TextureProperties
{
    ImageFormat format = ImageFormat::RGBA8;
    TextureWrap wrapping = TextureWrap::Repeat;
    TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureFilter mipMapFilter = TextureFilter::Linear;
    GLuint width = 0;
    GLuint height = 0;
    bool genMipmaps = true;
    bool srgb = true;
};

class Texture : public Resource
{
public:
    Texture() = default;
    Texture(const std::filesystem::path& path, bool srgb = true);
    ~Texture();

    void Bind() const;
    void Resize(GLuint width, GLuint height);
    void Clear(const glm::vec4& color);

    GLuint GetWidth() const { return mProperties.width; }
    GLuint GetHeight() const { return mProperties.height; }
    GLuint GetID() const { return mID; }
    ImageFormat GetImageFormat() const { return mProperties.format; }

private:
    GLuint LoadFromFile(const std::filesystem::path& path);
    GLuint InitializeTexture(const void* pixels, GLuint width, GLuint height);

private:
    TextureProperties mProperties;
    GLuint mID = 0;
};

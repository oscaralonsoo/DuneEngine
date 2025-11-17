#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <glad/glad.h>

enum class ShaderDataType
{
    None = 0,
    Bool,
    Int,
    Float,
    Vec2,
    Vec3,
    Vec4,
    IVec4,
    Mat2,
    Mat3,
    Mat4
};

static uint32_t ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
    case ShaderDataType::Bool:
        return 1;
    case ShaderDataType::Int:
        return 4;
    case ShaderDataType::Float:
        return 4;
    case ShaderDataType::Vec2:
        return 4 * 2;
    case ShaderDataType::Vec3:
        return 4 * 3;
    case ShaderDataType::Vec4:
        return 4 * 4;
    case ShaderDataType::IVec4:
        return 4 * 4;
    case ShaderDataType::Mat2:
        return 4 * 2 * 2;
    case ShaderDataType::Mat3:
        return 4 * 3 * 3;
    case ShaderDataType::Mat4:
        return 4 * 4 * 4;
    }

    return 0;
}

struct BufferAttribute
{
    std::string Name;
    ShaderDataType Type;
    uint32_t Size;
    size_t Offset;
    bool Normalized;

    BufferAttribute() = default;

    BufferAttribute(ShaderDataType type, const std::string &name, bool normalized = false)
        : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
    {
    }

    uint32_t GetComponentCount() const
    {
        switch (Type)
        {
        case ShaderDataType::Bool:
            return 1;
        case ShaderDataType::Int:
            return 1;
        case ShaderDataType::Float:
            return 1;
        case ShaderDataType::Vec2:
            return 2;
        case ShaderDataType::Vec3:
            return 3;
        case ShaderDataType::Vec4:
            return 4;
        case ShaderDataType::IVec4:
            return 4;
        case ShaderDataType::Mat2:
            return 2;
        case ShaderDataType::Mat3:
            return 3;
        case ShaderDataType::Mat4:
            return 4;
        }

        return 0;
    }
};

class BufferLayout
{
public:
    BufferLayout() {}

    BufferLayout(std::initializer_list<BufferAttribute> elements)
        : mAttributes(elements)
    {
        CalculateOffsetsAndStride();
    }

    uint32_t GetStride() const { return mStride; }

    const std::vector<BufferAttribute> &GetElements() const { return mAttributes; }

    std::vector<BufferAttribute>::iterator begin() { return mAttributes.begin(); }
    std::vector<BufferAttribute>::iterator end() { return mAttributes.end(); }
    std::vector<BufferAttribute>::const_iterator begin() const { return mAttributes.begin(); }
    std::vector<BufferAttribute>::const_iterator end() const { return mAttributes.end(); }

private:
    void CalculateOffsetsAndStride()
    {
        size_t offset = 0;
        mStride = 0;
        for (auto &attribute : mAttributes)
        {
            attribute.Offset = offset;
            offset += attribute.Size;
            mStride += attribute.Size;
        }
    }

private:
    std::vector<BufferAttribute> mAttributes;
    uint32_t mStride = 0;
};

class VertexBuffer
{
public:
    VertexBuffer(uint32_t size);
    VertexBuffer(float *vertices, uint32_t size);
    ~VertexBuffer();

    void Bind();
    void Unbind();
    void SetData(void *data, uint32_t size);

    const BufferLayout &GetLayout() const { return mLayout; }
    void SetLayout(const BufferLayout &layout) { mLayout = layout; }

private:
    GLuint mVBO = 0;
    BufferLayout mLayout;
};

class IndexBuffer
{
public:
    IndexBuffer(uint32_t *indices, uint32_t count);
    ~IndexBuffer();

    void Bind();
    void Unbind();
    GLuint GetCount() const { return mCount; }

private:
    GLuint mEBO;
    uint32_t mCount;
};
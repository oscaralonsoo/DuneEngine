#pragma once

#include <glad/glad.h>
#include <cstdint>
#include <vector>
#include <memory>

class VertexBuffer;
class IndexBuffer;

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void Bind();
    void Unbind();

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer);
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer);
    const std::vector<std::shared_ptr<VertexBuffer>> &GetVertexBuffers() const { return mVertexBuffers; }
    const std::shared_ptr<IndexBuffer> &GetIndexBuffer() const { return mIndexBuffer; }

private:
    GLuint mVAO;
    uint32_t mVertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> mVertexBuffers;
    std::shared_ptr<IndexBuffer> mIndexBuffer;
};
#include "VertexArray.h"
#include "Buffer.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &mVAO);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &mVAO);
}

void VertexArray::Bind()
{
    glBindVertexArray(mVAO);
}

void VertexArray::Unbind()
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer)
{
    glBindVertexArray(mVAO);
    vertexBuffer->Bind();

    const auto &layout = vertexBuffer->GetLayout();
    for (const auto &attribute : layout)
    {
        glEnableVertexAttribArray(mVertexBufferIndex);
        glVertexAttribPointer(mVertexBufferIndex,
                              attribute.GetComponentCount(),
                              GL_FLOAT,
                              attribute.Normalized ? GL_TRUE : GL_FALSE,
                              layout.GetStride(),
                              (const void *)attribute.Offset);
        mVertexBufferIndex++;
    }

    mVertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    glBindVertexArray(mVAO);
    indexBuffer->Bind();

    mIndexBuffer = indexBuffer;
}

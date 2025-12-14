#pragma once

#include <stdint.h>

class Framebuffer
{
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    void Bind();
    void Unbind();
    void Resize(uint32_t width, uint32_t height);
    uint32_t GetColorAttachment() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    uint32_t m_Width, m_Height;
    uint32_t m_RendererID;
    uint32_t m_ColorAttachment, m_DepthAttachment;
};

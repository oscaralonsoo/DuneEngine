#pragma once

#include <stdint.h>

class Framebuffer
{
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    void Bind();
    void Unbind();
};
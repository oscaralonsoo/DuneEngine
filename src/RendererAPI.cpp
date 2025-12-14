#include "RendererAPI.h"
#include "Globals.h"
#include "VertexArray.h"
#include "Buffer.h"
#include <glad/glad.h>

void RendererAPI::Init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST);
    
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

}

void RendererAPI::Clear(uint32_t clearFlags)
{
    uint32_t mask = 0;

    if (clearFlags & (uint32_t)ClearFlags::Color)
    {
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (clearFlags & (uint32_t)ClearFlags::Depth)
    {
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (clearFlags & (uint32_t)ClearFlags::Stencil)
    {
        mask |= GL_STENCIL_BUFFER_BIT;
    }
    if (mask == 0)
    {
        return;
    }
    glClear(mask);
}

void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
}

void RendererAPI::SetClearColor(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void RendererAPI::SetFaceCulling(bool enabled)
{
    if (enabled)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void RendererAPI::SetCullFace(CullFace face)
{
    switch (face)
    {
    case CullFace::Front:
        glCullFace(GL_FRONT);
        break;
    case CullFace::Back:
        glCullFace(GL_BACK);
        break;
    case CullFace::FrontAndBack:
        glCullFace(GL_FRONT_AND_BACK);
        break;
    default:
        LOG_WARN("Unknown CullFace enum value");
        break;
    }
}

void RendererAPI::SetPolygonMode(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::Fill:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case PolygonMode::Line:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case PolygonMode::Point:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    default:
        LOG_WARN("Unknown PolygonMode enum value");
        break;
    }
}

void RendererAPI::SetDepthMask(bool enabled)
{
    glDepthMask(enabled);
}

void RendererAPI::ClearDepth()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

void RendererAPI::SetDepthFunc(DepthFunc func)
{
    switch (func)
    {
        case DepthFunc::Less:
            glDepthFunc(GL_LESS);
            break;
        case DepthFunc::Lequal:
            glDepthFunc(GL_LEQUAL);
            break;
        default:
            glDepthFunc(GL_LESS);
            break;
    }
}


void RendererAPI::DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount)
{
    vertexArray->Bind();
    vertexArray->GetVertexBuffers()[0]->Bind();

    uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
    vertexArray->GetIndexBuffer()->Bind();

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void RendererAPI::DrawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t vertexCount, float lineWidth)
{
    vertexArray->Bind();
    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, vertexCount);
}
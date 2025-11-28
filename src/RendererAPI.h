#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

class VertexArray;

enum ClearFlags : uint32_t
{
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    ColorDepth = Color | Depth
};

enum class CullFace
{
    Front = 0,
    Back = 1,
    FrontAndBack = 2
};

enum class PolygonMode
{
    Fill = 0,
    Line = 1,
    Point = 2
};

enum DepthFunc
{
    Less = 0,
    Lequal = 1

};

class RendererAPI
{
public:
    static void Init();

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    static void SetClearColor(const glm::vec4 &color);
    static void Clear(uint32_t clearFlags = (uint32_t)ClearFlags::Color | (uint32_t)ClearFlags::Depth);
    static void SetColorMask(bool red, bool green, bool blue, bool alpha);
    static void SetDepthMask(bool enabled);
    static void ClearDepth();
    static void SetDepthFunc(DepthFunc func);
    static void SetFaceCulling(bool enabled);
    static void SetCullFace(CullFace face);
    static void SetPolygonMode(PolygonMode mode);
    static void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0);
    static void DrawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t vertexCount, float lineWidth = 1.0f);
};
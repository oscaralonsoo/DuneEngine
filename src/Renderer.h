#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Cubemap.h"

struct RenderObject
{
    glm::mat4 transform = glm::mat4(1.0f);
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    bool selected;
};

class Renderer
{
public:
    static void Init();

    static void ForwardPass(/*const std::shared_ptr<RenderTarget>& target*/);
    static void TransparentPass(/*const std::shared_ptr<RenderTarget>& target*/);
    static void SelectedPass(/*const std::shared_ptr<RenderTarget>& target*/);
    static void SkyboxPass(/*const std::shared_ptr<RenderTarget>& target*/);
    static void Submit(const RenderObject &renderObject);
    static void ResetRenderState();
    static std::shared_ptr<Shader> GetSingleColorShader() { return sSingleColorShader; }

private:
    static std::vector<RenderObject> sOpaqueRenderQueue;
    static std::vector<RenderObject> sTransparentRenderQueue;
    static std::vector<RenderObject> sSelectedRenderQueue;

    static  std::shared_ptr<Mesh> sSkyboxCube;
    static std::shared_ptr<Shader> sSkyboxShader;
    static std::shared_ptr<Shader> sSingleColorShader;
    static std::shared_ptr<Cubemap> sCubemap;

};
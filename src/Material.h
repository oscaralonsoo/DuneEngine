#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Material.h"

struct MaterialRenderSettings
{
    enum TransparencyMode
    {
        Disabled = 0,
        Alpha,
        AlphaCutoff
    } transparencyMode = TransparencyMode::Disabled;

    float alphaCutoff = 0.5f;

    // Blend mode
    // enum BlendMode
    // {
    //     Mix = 0,
    //     Add,
    //     Subtract,
    //     Multiply
    // } blendMode = BlendMode::Mix;

    // Culling
    // enum CullMode
    // {
    //     Front = 0,
    //     Back,
    //     None
    // } cullMode = CullMode::Back;

    bool depthTest = true;
    bool wireframe = false;
};

struct PBRMaterialProperties
{
    glm::vec4 color;
    float metallic = 0.0f;
    float roughness = 1.0f;
    float ao = 1.0f;
    glm::vec3 emissive;
};

struct PBRMaterialTextures
{
    std::shared_ptr<Texture> albedo;
    std::shared_ptr<Texture> normal;
    std::shared_ptr<Texture> metallic;
    std::shared_ptr<Texture> roughness;
    std::shared_ptr<Texture> ao;
    std::shared_ptr<Texture> emissive;
};

struct PBRMaterialTextureFlags
{
    bool hasAlbedo = false;
    bool hasNormal = false;
    bool hasMetallic = false;
    bool hasRoughness = false;
    bool hasAO = false;
    bool hasEmissive = false;
};

class Material : public Resource
{
public:
    Material() = default;
    Material(ResourceType type);
    Material(const std::string& name, PBRMaterialTextures& materialTextures);

    virtual ~Material() = default;

    MaterialRenderSettings &GetRenderSettings();

    std::shared_ptr<Shader> GetShader() const;
    void SetShader(std::shared_ptr<Shader> shader);

    void Use();

    PBRMaterialTextures &GetTextures();
    PBRMaterialProperties &GetProperties();
    const PBRMaterialTextures &GetTextures() const;
    const PBRMaterialProperties &GetProperties() const;

private:
    std::shared_ptr<Shader> mShader;
    MaterialRenderSettings mRenderSettings;

    PBRMaterialTextures mTextures;
    PBRMaterialTextureFlags mTextureFlags;
    PBRMaterialProperties mProperties;
    static std::shared_ptr<Texture> sMissingTexture;
    static std::shared_ptr<Shader> sStandardShader;
};
#include "Material.h"
#include "Globals.h"

std::shared_ptr<Texture> Material::sMissingTexture = nullptr;
std::shared_ptr<Shader> Material::sStandardShader = nullptr;

Material::Material(ResourceType type) : Resource(type)
{
    mName = "New Material";

    if (!sMissingTexture)
        //sMissingTexture = std::make_shared<Texture>("Assets/textures/missingTexture.jpg");

    if (!sStandardShader)
        sStandardShader = std::make_shared<Shader>("Assets/shaders/Shader.glsl");

    mProperties.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white color default

    mShader = sStandardShader;

    if (mShader)
    {
        mShader->Bind();
        mShader->SetInt("material_albedoMap", 0);
        mShader->Unbind();
    }
}

Material::Material(const std::string &name, PBRMaterialTextures &materialTextures)
{
    mName = name;

    if (!sStandardShader)
        sStandardShader = std::make_shared<Shader>("Assets/shaders/Shader.glsl");

    mTextures.albedo = materialTextures.albedo;
    mTextures.normal = materialTextures.normal;
    mTextures.metallic = materialTextures.metallic;
    mTextures.roughness = materialTextures.roughness;
    mTextures.ao = materialTextures.ao;
    mTextures.emissive = materialTextures.emissive;

    mTextureFlags.hasAlbedo = (mTextures.albedo != nullptr);
    mTextureFlags.hasNormal = (mTextures.normal != nullptr);
    mTextureFlags.hasMetallic = (mTextures.metallic != nullptr);
    mTextureFlags.hasRoughness = (mTextures.roughness != nullptr);
    mTextureFlags.hasAO = (mTextures.ao != nullptr);
    mTextureFlags.hasEmissive = (mTextures.emissive != nullptr);

    mProperties.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white color default

    if (mTextureFlags.hasMetallic)
        mProperties.metallic = 1.0f;
    if (mTextureFlags.hasEmissive)
        mProperties.emissive = glm::vec3(1.0f);

    mShader = sStandardShader;

    if (mShader)
    {
        mShader->Bind();
        int slot = 0;
        if (mTextureFlags.hasAlbedo)
            mShader->SetInt("material.albedoMap", slot++);
        if (mTextureFlags.hasNormal)
            mShader->SetInt("material.normalMap", slot++);
        if (mTextureFlags.hasMetallic)
            mShader->SetInt("material.metallicMap", slot++);
        if (mTextureFlags.hasRoughness)
            mShader->SetInt("material.roughnessMap", slot++);
        if (mTextureFlags.hasAO)
            mShader->SetInt("material.aoMap", slot++);
        if (mTextureFlags.hasEmissive)
            mShader->SetInt("material.emissiveMap", slot++);
        mShader->Unbind();
    }
}

void Material::Use()
{
    mTextureFlags.hasAlbedo = (mTextures.albedo != nullptr);
    mTextureFlags.hasNormal = (mTextures.normal != nullptr);
    mTextureFlags.hasMetallic = (mTextures.metallic != nullptr);
    mTextureFlags.hasRoughness = (mTextures.roughness != nullptr);
    mTextureFlags.hasAO = (mTextures.ao != nullptr);
    mTextureFlags.hasEmissive = (mTextures.emissive != nullptr);

    if (!mShader)
        return;

    mShader->Bind();

    int slot = 0;
    if (mTextureFlags.hasAlbedo)
    {
        mTextures.albedo->Bind(slot);
        mShader->SetInt("material_albedoMap", slot++);
    }
    if (mTextureFlags.hasNormal)
    {
        mTextures.normal->Bind(slot);
        mShader->SetInt("material_normalMap", slot++);
    }
    if (mTextureFlags.hasMetallic)
        mTextures.metallic->Bind(slot++);
    if (mTextureFlags.hasRoughness)
        mTextures.roughness->Bind(slot++);
    if (mTextureFlags.hasAO)
    {
        mTextures.ao->Bind(slot);
        mShader->SetInt("material_aoMap", slot++);
    }
    if (mTextureFlags.hasEmissive)
    {
        mTextures.emissive->Bind(slot);
        mShader->SetInt("material_emissiveMap", slot++);
    }

    mShader->SetVec4("material_color", mProperties.color);
    mShader->SetInt("material_hasAlbedo", mTextureFlags.hasAlbedo);
    mShader->SetInt("material_hasNormal", mTextureFlags.hasNormal);
    mShader->SetInt("material_hasEmissive", mTextureFlags.hasEmissive);
    mShader->SetInt("material_hasAO", mTextureFlags.hasAO);

    mShader->Unbind();
}

void Material::SetTexture(TextureType type, const std::shared_ptr<Texture> &texture)
{
    switch (type)
    {
    case TextureType::Albedo:
        mTextures.albedo = texture;
        break;
    case TextureType::Normal:
        mTextures.normal = texture;
        break;
    case TextureType::Metallic:
        mTextures.metallic = texture;
        break;
    case TextureType::Roughness:
        mTextures.roughness = texture;
        break;
    case TextureType::AO:
        mTextures.ao = texture;
        break;
    case TextureType::Emissive:
        mTextures.emissive = texture;
        break;
    }

    mTextureFlags.hasAlbedo = (mTextures.albedo != nullptr);
    mTextureFlags.hasNormal = (mTextures.normal != nullptr);
    mTextureFlags.hasMetallic = (mTextures.metallic != nullptr);
    mTextureFlags.hasRoughness = (mTextures.roughness != nullptr);
    mTextureFlags.hasAO = (mTextures.ao != nullptr);
    mTextureFlags.hasEmissive = (mTextures.emissive != nullptr);
}

MaterialRenderSettings &Material::GetRenderSettings()
{
    return mRenderSettings;
}

std::shared_ptr<Shader> Material::GetShader() const
{
    return mShader;
}

void Material::SetShader(std::shared_ptr<Shader> shader)
{
    mShader = shader;
}

PBRMaterialTextures &Material::GetTextures()
{
    return mTextures;
}

PBRMaterialProperties &Material::GetProperties()
{
    return mProperties;
}

#include "Material.h"
#include "Globals.h"

std::shared_ptr<Texture> Material::sMissingTexture = nullptr;
std::shared_ptr<Shader> Material::sStandardShader = nullptr;

Material::Material(ResourceType type) : Resource(type)
{
    if (!sMissingTexture)
        sMissingTexture = std::make_shared<Texture>("Assets/textures/missingTexture.jpg");

    if (!sStandardShader)
        sStandardShader = std::make_shared<Shader>("Assets/shaders/Shader.glsl");

    mTextures.albedo = sMissingTexture;
    mTextureFlags.hasAlbedo = true;

    mShader = sStandardShader;

    if (mShader)
    {
        mShader->Bind();
        if (mTextures.albedo)
            mTextures.albedo->Bind(0);
        mShader->SetInt("material.albedoMap", 0);
        mShader->Unbind();
    }
}

Material::Material(const std::string &name, PBRMaterialTextures &materialTextures)
{
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
        mTextures.albedo->Bind(slot++);
    if (mTextureFlags.hasNormal)
        mTextures.normal->Bind(slot++);
    if (mTextureFlags.hasMetallic)
        mTextures.metallic->Bind(slot++);
    if (mTextureFlags.hasRoughness)
        mTextures.roughness->Bind(slot++);
    if (mTextureFlags.hasAO)
        mTextures.ao->Bind(slot++);
    if (mTextureFlags.hasEmissive)
        mTextures.emissive->Bind(slot++);

    mShader->SetVec4("material.color", mProperties.color);
    mShader->SetFloat("material.metallic", mProperties.metallic);
    mShader->SetFloat("material.roughness", mProperties.roughness);
    mShader->SetFloat("material.ao", mProperties.ao);
    mShader->SetVec3("material.emissive", mProperties.emissive);

    mShader->SetInt("material.hasAlbedo", mTextureFlags.hasAlbedo);
    mShader->SetInt("material.hasNormal", mTextureFlags.hasNormal);
    mShader->SetInt("material.hasMetallic", mTextureFlags.hasMetallic);
    mShader->SetInt("material.hasRoughness", mTextureFlags.hasRoughness);
    mShader->SetInt("material.hasAO", mTextureFlags.hasAO);
    mShader->SetInt("material.hasEmissive", mTextureFlags.hasEmissive);

    mShader->SetInt("material.transparencyMode", mRenderSettings.transparencyMode);
    mShader->SetFloat("material.alphaCutoff", mRenderSettings.alphaCutoff);

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

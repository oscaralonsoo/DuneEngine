#pragma once

#include "Resource.h"
#include <glm/glm.hpp>

class Shader : public Resource
{
public:
    Shader() = default;
    Shader(const std::filesystem::path &shaderPath);
    ~Shader();
    
    void Bind();
    void Unbind();

    void SetBool(const std::string &name, bool value) const;
    void SetInt(const std::string &name, int value) const;
    void SetFloat(const std::string &name, float value) const;
    void SetVec2(const std::string &name, const glm::vec2 &value) const;
    void SetVec3(const std::string &name, const glm::vec3 &value) const;
    void SetVec4(const std::string &name, const glm::vec4 &value) const;
    void SetMat2(const std::string &name, const glm::mat2 &mat) const;
    void SetMat3(const std::string &name, const glm::mat3 &mat) const;
    void SetMat4(const std::string &name, const glm::mat4 &mat) const;

    void CheckCompileErrors(unsigned int shader, std::string type);

private:
    std::string ReadShaderFile(const std::filesystem::path &shaderPath);
    void CompileShader(const std::string &shaderSource);
    void InitializeShader(const std::filesystem::path &shaderPath);

private:
    unsigned int mID = 0;
};
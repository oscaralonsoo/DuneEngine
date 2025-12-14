#include "Shader.h"
#include "Globals.h"

#include <glad/glad.h>

#include <fstream>
#include <sstream>

Shader::Shader(const std::filesystem::path &shaderPath)
    : Resource(ResourceType::Shader)
{
    InitializeShader(shaderPath);
}

Shader::~Shader()
{
    glDeleteProgram(mID);
}

void Shader::Bind()
{
    glUseProgram(mID);
}

void Shader::Unbind()
{
    glUseProgram(0);
}

void Shader::SetBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(mID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::SetMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

std::string Shader::ReadShaderFile(const std::filesystem::path &shaderPath)
{
    std::string shaderCode;
    std::ifstream shaderFile;

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        LOG_ERROR("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: {}", e.what());
    }

    return shaderCode;
}

void Shader::CheckCompileErrors(unsigned int shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            LOG_ERROR("ERROR::SHADER_COMPILATION_ERROR of type: {0}\n{1}\n", type, infoLog);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            LOG_ERROR("ERROR::PROGRAM_LINKING_ERROR of type: {0}\n{1}\n", type, infoLog);
        }
    }
}

void Shader::CompileShader(const std::string &shaderSource)
{
    const std::string vertexDelimiter = "#[vertex]";
    const std::string fragmentDelimiter = "#[fragment]";

    size_t vertexPos = shaderSource.find(vertexDelimiter);
    size_t fragmentPos = shaderSource.find(fragmentDelimiter);

    if (vertexPos == std::string::npos || fragmentPos == std::string::npos)
    {
        LOG_ERROR("ERROR::SHADER::DELIMITER_NOT_FOUND: Delimiter not found in shader file!");
        return;
    }

    std::string vertexCode = shaderSource.substr(vertexPos + vertexDelimiter.length(), fragmentPos - vertexPos - vertexDelimiter.length());
    std::string fragmentCode = shaderSource.substr(fragmentPos + fragmentDelimiter.length(), shaderSource.length() - fragmentPos - fragmentDelimiter.length());

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    mID = glCreateProgram();
    glAttachShader(mID, vertex);
    glAttachShader(mID, fragment);
    glLinkProgram(mID);
    CheckCompileErrors(mID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::InitializeShader(const std::filesystem::path &shaderPath)
{
    mName = shaderPath.filename().string();
    std::string shaderCode = ReadShaderFile(shaderPath);
    CompileShader(shaderCode);
}

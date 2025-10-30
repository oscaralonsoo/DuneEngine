#pragma once
#include <string>
#include <glad/glad.h>

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool compile(const char* vsSrc, const char* fsSrc);
    void use() const { glUseProgram(m_program); }
    GLuint id() const { return m_program; }

private:
    bool compileStage(GLuint sh, const char* src, const char* tag);
    GLuint m_program = 0;
};

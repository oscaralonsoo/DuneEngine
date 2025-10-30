#include "Shader.h"
#include <SDL3/SDL.h>

Shader::~Shader() {
    if (m_program) glDeleteProgram(m_program);
}

bool Shader::compile(const char* vsSrc, const char* fsSrc) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    if (!compileStage(vs, vsSrc, "VERTEX")) return false;

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileStage(fs, fsSrc, "FRAGMENT")) { glDeleteShader(vs); return false; }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint linked; glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024]; glGetProgramInfoLog(m_program, 1024, nullptr, log);
        SDL_Log("ERROR::SHADER::PROGRAM::LINK_FAILED\n%s", log);
        glDeleteShader(vs); glDeleteShader(fs); glDeleteProgram(m_program); m_program = 0;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

bool Shader::compileStage(GLuint sh, const char* src, const char* tag) {
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(sh, 1024, nullptr, log);
        SDL_Log("ERROR::SHADER::%s::COMPILE_FAILED\n%s", tag, log);
        glDeleteShader(sh);
        return false;
    }
    return true;
}

#pragma once
#include <glad/glad.h>

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    // Crea un quad con índice (tu rectángulo 2 triángulos)
    void createQuad();
    void bind() const;
    void draw() const;

private:
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    GLsizei m_indexCount = 0;
};

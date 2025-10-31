#include "Mesh.h"

Mesh::~Mesh() {
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Mesh::createQuad() {
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
       -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 0.0f,
       -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f  
    };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };
    m_indexCount = 6;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::CreateCube(){
    const float radius = 0.5f; //la mitad de la distancia de cada arista para hacer que el cuadrado mida 1,1,1
    const float vertices[] = {
        -v, -v, -v,  1,0,0,
         v, -v, -v,  0,1,0,
         v,  v, -v,  0,0,1,
        -v,  v, -v,  1,1,0,
        -v, -v,  v,  1,0,1,
         v, -v,  v,  0,1,1,
         v,  v,  v,  1,1,1,
        -v,  v,  v,  0.2f,0.6f,1.0f
    };

    const unsigned int indices[] = {
        0,1,2,  0,2,3,
        4,6,5,  4,7,6,
        0,3,7,  0,7,4,
        1,5,6,  1,6,2,
        0,4,5,  0,5,1,
        3,2,6,  3,6,7
    };
    m_indexCount = 36;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::bind() const {
    glBindVertexArray(m_vao);
}

void Mesh::draw() const {
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
}

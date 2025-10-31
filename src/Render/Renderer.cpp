#include "Renderer.h"
#include <SDL3/SDL.h>

void Renderer::clear(float r, float g, float b, float a) {
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderMesh(const Mesh& mesh, Shader& shader, RenderMode mode) {
    if (mode == RenderMode::Solid) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        shader.use();
        shader.setVec3("uTint", 1.0f, 1.0f, 1.0f);
        mesh.bind();
        mesh.draw();
        return;
    }

    // Wireframe con “líneas ocultas”
    glLineWidth(1.5f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader.use();
    mesh.bind();

    // Pasada trasera (tenue), sin depth
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    shader.setVec3("uTint", 0.25f, 0.25f, 0.25f);
    mesh.draw();

    // Pasada frontal (blanca), con depth
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    shader.setVec3("uTint", 1.0f, 1.0f, 1.0f);
    mesh.draw();

    // (Opcional) restaurar fill aquí si luego lo necesitas
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

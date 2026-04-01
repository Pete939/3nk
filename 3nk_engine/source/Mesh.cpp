#include "3nk/Mesh.hpp"

#include <iostream>

#include "3nk/glError.hpp"


Mesh::Mesh()  {}

void Mesh::Render(Shader &shader) {
    // shader.use();
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
}

void Mesh::InitOpenGLBuffers() {
    uint32_t vbo, vboUV, vboCol, vboNor, vboBone, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    if (!texCoords.empty()) {
        glGenBuffers(1, &vboUV);
        glBindBuffer(GL_ARRAY_BUFFER, vboUV);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
    }

    if (!colors.empty()) {
        glGenBuffers(1, &vboCol);
        glBindBuffer(GL_ARRAY_BUFFER, vboCol);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
    }
    else {
        glDisableVertexAttribArray(2);
        glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f); // white
    }

    if (!normals.empty()) {
        glGenBuffers(1, &vboNor);
        glBindBuffer(GL_ARRAY_BUFFER, vboNor);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
        glEnableVertexAttribArray(3);
    }

    if (!vertexToBones.empty()) {
        glGenBuffers(1, &vboBone);
        glBindBuffer(GL_ARRAY_BUFFER, vboBone);
        glBufferData(GL_ARRAY_BUFFER, vertexToBones.size() * sizeof(VertexToBone), vertexToBones.data(), GL_STATIC_DRAW);
        glVertexAttribIPointer(4, 4, GL_INT, sizeof(VertexToBone), (void*)0);
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 4, GL_FLOAT, GL_TRUE,  sizeof(VertexToBone), (void*)(4 * sizeof(int32_t)));
        glEnableVertexAttribArray(5);
    }

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);


    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
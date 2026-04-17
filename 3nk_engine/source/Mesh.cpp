#include "3nk/Mesh.hpp"

#include <iostream>

#include "3nk/glError.hpp"


Mesh::Mesh()  {}

void Mesh::Render(Shader &shader) {
    // shader.use();
    glBindVertexArray(m_Vao);
    glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_SHORT, 0);
}

void Mesh::InitOpenGLBuffers() {
    uint32_t vbo, vboUV, vboCol, vboNor, vboBone, ebo;

    mNumIndices = mIndices.size();

    glGenVertexArrays(1, &m_Vao);
    glBindVertexArray(m_Vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), mVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    mVertices.clear();

    if (!mTexCoords.empty()) {
        glGenBuffers(1, &vboUV);
        glBindBuffer(GL_ARRAY_BUFFER, vboUV);
        glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(float), mTexCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        mTexCoords.clear();
    }

    if (!mColors.empty()) {
        glGenBuffers(1, &vboCol);
        glBindBuffer(GL_ARRAY_BUFFER, vboCol);
        glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(float), mColors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
        mColors.clear();
    }
    else {
        glDisableVertexAttribArray(2);
        glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f); // white
    }

    if (!mNormals.empty()) {
        glGenBuffers(1, &vboNor);
        glBindBuffer(GL_ARRAY_BUFFER, vboNor);
        glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), mNormals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
        glEnableVertexAttribArray(3);
        mNormals.clear();
    }

    if (!mVertexToBones.empty()) {
        glGenBuffers(1, &vboBone);
        glBindBuffer(GL_ARRAY_BUFFER, vboBone);
        glBufferData(GL_ARRAY_BUFFER, mVertexToBones.size() * sizeof(VertexToBone), mVertexToBones.data(), GL_STATIC_DRAW);
        glVertexAttribIPointer(4, 4, GL_INT, sizeof(VertexToBone), (void*)0);
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 4, GL_FLOAT, GL_TRUE,  sizeof(VertexToBone), (void*)(4 * sizeof(int32_t)));
        glEnableVertexAttribArray(5);
        mVertexToBones.clear();
    }

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumIndices * sizeof(uint16_t), mIndices.data(), GL_STATIC_DRAW);
    mIndices.clear();


    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
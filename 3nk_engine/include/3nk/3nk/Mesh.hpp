#pragma once

#include <cstdint>
#include <vector>

#include "Shader.h"

constexpr int MAX_VERTEX_BONES = 4;
struct VertexToBone {
    int32_t boneIds[MAX_VERTEX_BONES]{-1, -1, -1, -1};
    float weights[MAX_VERTEX_BONES]{0.0f};
};


class Mesh {
public:
    Mesh();
    void InitOpenGLBuffers();
    void Render(Shader &shader);

    uint32_t vao;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<uint16_t> indices;
    std::vector<float> colors;
    std::vector<VertexToBone> vertexToBones;
    int materialIndex = -1;
};

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
    uint32_t m_Vao;
public:
    Mesh();
    void InitOpenGLBuffers();
    void Render(Shader &shader);

    std::vector<float> mVertices;
    std::vector<float> mNormals;
    std::vector<float> mTexCoords;
    std::vector<uint16_t> mIndices;
    std::vector<float> mColors;
    std::vector<VertexToBone> mVertexToBones;
    int mMaterialIndex = -1;
    int mNumIndices = 0;
};

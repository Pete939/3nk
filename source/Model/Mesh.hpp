#pragma once

#include <cstdint>
#include <vector>

#include "../Shader/Shader.h"

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
    int materialIndex;
private:
};

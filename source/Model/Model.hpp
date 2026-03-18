#pragma once

#include <cstdint>
#include <vector>
#include "Mesh.hpp"
#include "Node.hpp"
#include "../Texture/Texture.h"

class Model {
public:
    Model();
    void Render(Shader &shader);
    void InitOpenGLBuffers();

    std::string directory;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
};

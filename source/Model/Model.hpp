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
    void SetTransformMatrix(const glm::mat4 &mat);
    std::string directory;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
private:
    void AddRootNode();
    void ProcessNode(size_t idx, glm::mat4 transform, Shader &shader);
    uint32_t rootNodeIdx;
};

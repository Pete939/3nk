
#include "Model.hpp"

#include <iostream>
#include <fmt/core.h>

Model::Model() {}

void Model::Render(Shader &shader) {
    if (!textures.empty()) {
        textures[0].use(0);
    }
    shader.use();
    for (auto & node : nodes) {
        shader.setMat4("model", node.transform);
        meshes[node.meshIndex].Render(shader);
    }
}

void Model::InitOpenGLBuffers() {
    for (auto & mesh : meshes) {
        mesh.InitOpenGLBuffers();
    }
    if (!textures.empty()) {
        std::string path = directory + textures[0].path;
        textures[0].load(path);
    }
}


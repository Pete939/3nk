
#include "Model.hpp"
#include "WhiteTexture.hpp"

#include <iostream>
#include <fmt/core.h>



Model::Model() {

}

void Model::ProcessNode(size_t idx, glm::mat4 transform, Shader &shader) {
    auto &node = nodes[idx];
    if (!node.childrenIndices.empty()) {
        for (uint32_t index : node.childrenIndices) {
            transform = transform * nodes[index].transform;
            ProcessNode(index, transform, shader);
        }
    }
    shader.setMat4("model", transform);
    if (node.meshIndex >= 0) {
        auto &mesh = meshes[node.meshIndex];
        if (mesh.materialIndex >= 0) {
            // TODO: Keep n=0?
            textures[mesh.materialIndex].use(0);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, GetWhiteTexture());
        }
        mesh.Render(shader);
    }
}

void Model::SetTransformMatrix(const glm::mat4 &mat) {
    nodes[rootNodeIdx].transform = mat;
}

// Store root-level model's nodes into artificial root node
void Model::AddRootNode() {
    assert(!nodes.empty());
    Node rootNode;
    rootNode.parentIndex = -1;
    rootNode.meshIndex = -1;
    // Each node that does NOT have a parent is considered a root node;
    for (uint32_t i = 0; i < nodes.size(); ++i) {
        const auto &node = nodes[i];
        if (node.parentIndex == -1) {
            rootNode.childrenIndices.push_back(i);
        }
    }
    nodes.push_back(rootNode);
    rootNodeIdx = nodes.size() - 1;
}

void Model::Render(Shader &shader) {
    shader.use();
    ProcessNode(rootNodeIdx, nodes[rootNodeIdx].transform, shader);
}

void Model::InitOpenGLBuffers() {
    for (auto & mesh : meshes) {
        mesh.InitOpenGLBuffers();
        if (mesh.materialIndex >= 0) {
            std::string path = directory + textures[mesh.materialIndex].path;
            textures[mesh.materialIndex].load(path);
        }
    }
    AddRootNode();
}


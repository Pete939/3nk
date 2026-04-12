#pragma once

#include <vector>

#include "Mesh.hpp"
#include "Node.hpp"
#include "Texture.h"
#include "Animator.hpp"

class Model {
public:
    Model();
    // void Render(Shader &shader);
    void AssimpRender(Shader &shader, float dt, bool normalRender);

    void InitOpenGLBuffers();
    // void SetTransformMatrix(const glm::mat4 &mat);
    std::string directory;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    Animator *mAnimator = nullptr;
    // glm::mat4 globalInverseTransform{1.0f};
private:
    void AssimpNodeRender(size_t idx, glm::mat4 transform, Shader &shader, float dt);
    uint32_t rootNodeIdx;
};
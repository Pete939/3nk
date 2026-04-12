
#include "3nk/Model.hpp"
#include "3nk/WhiteTexture.hpp"

#include <fmt/core.h>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>


Model::Model() {

}

void Model::AssimpNodeRender(size_t idx, glm::mat4 transform, Shader &shader,float dt) {
    auto &node = nodes[idx];
    if (!node.childrenIndices.empty()) {
        for (uint32_t index : node.childrenIndices) {
            transform = transform * nodes[index].transform;
            AssimpNodeRender(index, transform, shader, dt);
        }
    }
    // shader.setMat4("model", transform);
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



void Model::AssimpRender(Shader &shader, float dt, bool normalRender=false) {
    assert(mAnimator != nullptr);
    mAnimator->Update(dt);

    std::string uniBoneArrName("boneMatrices");
    shader.setMat4s(uniBoneArrName, 100, mAnimator->mBoneMatrices.data());

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -5.0f));
    shader.setMat4("model", model);

    meshes[0].Render(shader);
    // AssimpNodeRender(0, glm::mat4{1.0f}, shader, dt);
}

void Model::InitOpenGLBuffers() {
    for (auto & mesh : meshes) {
        mesh.InitOpenGLBuffers();
    }
}



#include "3nk/Model.hpp"
#include "3nk/WhiteTexture.hpp"

#include <fmt/core.h>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>


Model::Model() {

}

void Model::AssimpNodeRender(size_t idx, glm::mat4 transform, Shader &shader,float dt) {
    auto &node = mNodes[idx];
    if (!node.mChildrenIndices.empty()) {
        for (uint32_t index : node.mChildrenIndices) {
            transform = transform * mNodes[index].mTransform;
            AssimpNodeRender(index, transform, shader, dt);
        }
    }
    shader.SetMat4("model", node.mTransform * transform);
    if (node.mMeshIndex >= 0) {
        auto &mesh = mMeshes[node.mMeshIndex];
        if (mesh.mMaterialIndex >= 0) {
            // TODO: Keep n=0?
            mTextures[mesh.mMaterialIndex].Use(0);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, GetWhiteTexture());
        }
        mesh.Render(shader);
    }
}



void Model::AssimpRender(Shader &shader, const glm::mat4 &transform, float dt) {
    if (mAnimator != nullptr) {
        mAnimator->Update(dt);
        std::string uniBoneArrName("boneMatrices");
        shader.SetMat4s(uniBoneArrName, 100, mAnimator->mBoneMatrices.data());
        AssimpNodeRender(0, transform, shader, dt);
        return;
    }
    AssimpNodeRender(0, transform, shader, dt);
}

void Model::InitOpenGLBuffers() {
    for (auto & mesh : mMeshes) {
        mesh.InitOpenGLBuffers();
    }
}


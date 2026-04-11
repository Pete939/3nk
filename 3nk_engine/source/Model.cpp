
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

void Model::CalculateBones(size_t nodeId, glm::mat4 transform, float dt) {
    auto &node = nodes[nodeId];
    glm::mat4 nodeT = node.transform;
    // Get bone
    if (node.animNodeIndex != -1) {
        // TODO: Separate index for anim nodes?
        auto &bone = animationNodes[node.animNodeIndex];
        auto t = glm::vec3(0.0f);
        auto s = glm::vec3(1.0f);
        auto r = glm::identity<glm::quat>();
        // FIXME: If no key fits then set it to the last one(?)
        if (!bone.positionKeys.empty()) {
            bool found = false;
            for (size_t i = 0; i < bone.positionKeys.size(); ++i) {
                if (dt < bone.positionKeys[i].timeStamp / 1000.0f) {
                    t = bone.positionKeys[i].position;
                    found = true;
                    break;
                }
            }
            if (!found) {
                t = bone.positionKeys[bone.positionKeys.size() - 1].position;
            }
        }
        if (!bone.rotationKeys.empty()) {
            bool found = false;
            for (size_t i = 0; i < bone.rotationKeys.size(); ++i) {
                if (dt < bone.rotationKeys[i].timeStamp / 1000.0f) {
                    r = bone.rotationKeys[i].rotation;
                    r = glm::normalize(r);
                    found = true;
                    break;
                }
            }
            if (!found) {
                r = bone.rotationKeys[bone.rotationKeys.size() - 1].rotation;
            }
        }
        if (!bone.scalingKeys.empty()) {
            bool found = false;
            for (size_t i = 0; i < bone.scalingKeys.size(); ++i) {
                if (dt < bone.scalingKeys[i].timeStamp / 1000.0f) {
                    s = bone.scalingKeys[i].scale;
                    found = true;
                    break;
                }
            }
            if (!found) {
                s = bone.scalingKeys[bone.scalingKeys.size() - 1].scale;
            }
        }

        glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
        glm::mat4 R = glm::mat4_cast(r); // quaternion → mat4
        glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
        nodeT = T * R * S;
    }

    glm::mat4 globalT = transform * nodeT;

    if (node.boneIndex != -1) {
        auto bi = boneInfos[node.boneIndex];
        boneMatrices[node.boneIndex] = globalT * bi.mOffsetMatrix;
    }

    for (size_t i = 0; i < node.childrenIndices.size(); ++i) {
        CalculateBones(node.childrenIndices[i], globalT, dt);
    }
}

void Model::AssimpRender(Shader &shader, float dt, bool normalRender=false) {
    animTime += dt;
    // constexpr float ANIMATION_TIME = 2.0f;
    animTime = fmod(animTime, animDuration);
    CalculateBones(0, glm::mat4{1.0f}, animTime);

    std::string uniBoneArrName("boneMatrices");
    shader.setMat4s(uniBoneArrName, 100, boneMatrices.data());

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


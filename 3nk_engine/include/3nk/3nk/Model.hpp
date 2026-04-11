#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <glm/detail/type_quat.hpp>

#include "Mesh.hpp"
#include "Node.hpp"
#include "Texture.h"

struct BoneInfo {
    glm::mat4 mOffsetMatrix;
    glm::mat4 mFinalTransformation;
};

struct PositionKey {
    double timeStamp;
    glm::vec3 position;
};

struct ScalingKey {
    double timeStamp;
    glm::vec3 scale;
};

struct RotationKey {
    double timeStamp;
    glm::quat rotation;
};

struct AnimationNode {
    std::vector<PositionKey> positionKeys;
    std::vector<ScalingKey> scalingKeys;
    std::vector<RotationKey> rotationKeys;
};

class Model {
public:
    Model();
    void Render(Shader &shader);
    void AssimpRender(Shader &shader, float dt, bool normalRender);

    void InitOpenGLBuffers();
    void SetTransformMatrix(const glm::mat4 &mat);
    std::string directory;
    std::vector<Node> nodes;
    std::vector<AnimationNode> animationNodes;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    std::vector<BoneInfo> boneInfos;
    glm::mat4 globalInverseTransform{1.0f};
    float animTime = 0.0f;
    float animDuration = 0.0f;
    std::array<glm::mat4, 100> boneMatrices;
    int numBones = 0;
    int numAnimationNodes = 0;
private:
    void AssimpNodeRender(size_t idx, glm::mat4 transform, Shader &shader, float dt);
    void CalculateBones(size_t nodeId, glm::mat4 transform, float dt);
    void AddRootNode();
    void ProcessNode(size_t idx, glm::mat4 transform, Shader &shader);
    uint32_t rootNodeIdx;
};
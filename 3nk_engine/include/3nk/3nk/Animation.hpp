#pragma once
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>

struct BoneInfo;
struct PositionKey;
struct RotationKey;
struct ScalingKey;
struct AnimationNode;


class Animation {
public:
    float mDuration;
    int32_t mNumBones = 0;
    std::vector<AnimationNode> mNodes;
    std::vector<BoneInfo> mBoneInfos;
};

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

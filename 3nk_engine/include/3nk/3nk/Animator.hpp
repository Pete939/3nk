#pragma once

#include <array>
#include <cmath>
#include <vector>
#include <3nk/Animation.hpp>
#include <fmt/core.h>
#include <glm/gtc/quaternion.hpp>

#include "Node.hpp"

constexpr int MAX_JOINTS = 100;

class Animator {
public:
    int32_t mCurrentAnimationIndex = -1;
    int32_t mNumBones = 0;
    std::vector<Node> *mModelNodes = nullptr;
    std::vector<Animation> mAnimations;
    float mCurrentAnimationTime = 0.0f;
    std::array<glm::mat4, MAX_JOINTS> mBoneMatrices;
    std::vector<BoneInfo> mBoneInfos;
    bool mAnimationPaused = false;
    void Update(float dt) {
        if (mCurrentAnimationIndex != -1) {
            mAnimationPaused = false;
            const auto &anim = mAnimations[mCurrentAnimationIndex];
            mCurrentAnimationTime += dt;
            mCurrentAnimationTime = std::fmod(mCurrentAnimationTime, anim.mDuration);
            CalculateBones(0, glm::mat4{1.0f}, mCurrentAnimationTime);
        }
        else {
            if (mAnimationPaused) {
                return;
            }
            for (auto &mat : mBoneMatrices) {
                mat = glm::mat4{1.0f};
            }
            mAnimationPaused = true;
        }
    }
    void PlayAnimation(int id) {
        // if (id < 0 or id >= mAnimations.size()) {
            // fmt::print("Trying to play non-existent animation!\n");
            // return;
        // }
        mCurrentAnimationIndex = id;
        mCurrentAnimationTime = 0.0f;
    }



    void CalculateBones(size_t nodeId, glm::mat4 transform, float dt) {
        assert(mModelNodes != nullptr);
        auto &node = (*mModelNodes)[nodeId];
        glm::mat4 nodeT = node.mTransform;
        // Get bone
        if (node.mAnimNodeIndex != -1) {
            auto &bone = mAnimations[mCurrentAnimationIndex].mNodes[node.mAnimNodeIndex];
            auto t = glm::vec3(0.0f);
            auto s = glm::vec3(1.0f);
            auto r = glm::identity<glm::quat>();
            if (!bone.positionKeys.empty()) {
                t = InterpolatePosition(bone, dt);
            }
            if (!bone.rotationKeys.empty()) {
                r = InterpolateRotation(bone, dt);
            }
            if (!bone.scalingKeys.empty()) {
                s = InterpolateScaling(bone, dt);
            }

            glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
            glm::mat4 R = glm::mat4_cast(r); // quaternion → mat4
            glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
            nodeT = T * R * S;
        }

        glm::mat4 globalT = transform * nodeT;

        if (node.mBoneIndex != -1) {
            auto bi = mBoneInfos[node.mBoneIndex];
            mBoneMatrices[node.mBoneIndex] = globalT * bi.offsetMatrix;
        }

        for (size_t i = 0; i < node.mChildrenIndices.size(); ++i) {
            CalculateBones(node.mChildrenIndices[i], globalT, dt);
        }
    }

private:
    glm::vec3 InterpolatePosition(const AnimationNode &bone, float dt) {
        if (bone.positionKeys.size() == 1) {
            return bone.positionKeys[0].position;
        }
        const int p0_index = GetPositionKeyIndex(bone, dt);
        const int p1_index = p0_index + 1;
        const auto &p0 = bone.positionKeys[p0_index];
        const auto &p1 = bone.positionKeys[p1_index];
        float scale_factor = GetScaleFactor(p0.timeStamp, p1.timeStamp, dt);
        return glm::mix(p0.position, p1.position, scale_factor);
    }

    glm::quat InterpolateRotation(const AnimationNode &bone, float dt) {
        if (bone.rotationKeys.size() == 1) {
            return bone.rotationKeys[0].rotation;
        }
        const int r0_index = GetRotationKeyIndex(bone, dt);
        const int r1_index = r0_index + 1;
        const auto &r0 = bone.rotationKeys[r0_index];
        const auto &r1 = bone.rotationKeys[r1_index];
        float scale_factor = GetScaleFactor(r0.timeStamp, r1.timeStamp, dt);
        // FIXME: Quaternion shortest path issue - edge case: r0 and r1 are on opposite hemispheres
        return glm::slerp(r0.rotation, r1.rotation, scale_factor);
    }

    glm::vec3 InterpolateScaling(const AnimationNode &bone, float dt) {
        if (bone.scalingKeys.size() == 1) {
            return bone.scalingKeys[0].scale;
        }
        assert(bone.scalingKeys.size() >= 2);
        const int s0_index = GetScalingKeyIndex(bone, dt);
        const int s1_index = s0_index + 1;
        const auto &p0 = bone.scalingKeys[s0_index];
        const auto &p1 = bone.scalingKeys[s1_index];
        float scale_factor = GetScaleFactor(p0.timeStamp, p1.timeStamp, dt);
        return glm::mix(p0.scale, p1.scale, scale_factor);
    }

    int32_t GetPositionKeyIndex(const AnimationNode &bone, float dt) {
        for (size_t i = 0; i < bone.positionKeys.size(); ++i) {
            if (dt < bone.positionKeys[i + 1].timeStamp) {
                return i;
            }
        }
        assert(0);
    }

    int32_t GetRotationKeyIndex(const AnimationNode &bone, float dt) {
        for (size_t i = 0; i < bone.rotationKeys.size() - 1; ++i) {
            if (dt < bone.rotationKeys[i + 1].timeStamp) {
                return i;
            }
        }
        assert(0);
    }

    int32_t GetScalingKeyIndex(const AnimationNode &bone, float dt) {
        for (size_t i = 0; i < bone.scalingKeys.size(); ++i) {
            if (dt < bone.scalingKeys[i + 1].timeStamp) {
                return i;
            }
        }
        assert(0);
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        if (framesDiff == 0.0f) {
            return 0.0f;
        }
        float scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }
};

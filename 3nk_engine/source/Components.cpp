
#include "3nk/Components.hpp"
#include "imgui.h"

Component::Component() { mActive = true; }

glm::mat4 CTransform::Matrix() const {
    const glm::mat4 matrix = glm::translate(glm::mat4{1.0f}, mPosition);
    return glm::scale(matrix, mScale);
}

void CTransform::Start() {}

void CTransform::Update(float dt) {}

void CTransform::OnInspector() {
    ImGui::InputFloat3("Position", &mPosition.x);
    ImGui::InputFloat("Scale", &mScale.x);
    mScale.y = mScale.z = mScale.x;
}

// CRender::CRender() {}

void CRender::Start() {}

void CRender::Update(float dt) {}

void CRender::OnInspector() {
    ImGui::Checkbox("Visible", &mVisible);
    ImGui::Checkbox("Show Triangles", &mShowTriangles);
    if (mModel.mAnimator->mNumBones <= 0) {
        return;
    }

    ImGui::Checkbox("Show Bones", &mShowBones);
    int maxValue = mModel.mAnimator->mNumBones;
    if (mShowBones) {
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
            mSelectedBoneId = (mSelectedBoneId - 1 + maxValue + 1) % (maxValue + 1);
        }

        ImGui::SameLine();
        ImGui::Text("%d", mSelectedBoneId);
        ImGui::SameLine();

        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
            mSelectedBoneId = (mSelectedBoneId + 1) % (maxValue + 1);
        }
        ImGui::PopButtonRepeat();
    }

}
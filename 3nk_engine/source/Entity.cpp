
#include "3nk/Entity.hpp"
#include "3nk/EntityPool.hpp"
#include "imgui.h"

Entity::Entity(const uint32_t id) : m_Id(id) {}

void Entity::OnInspector() {
    Get<CTransform>().OnInspector();
    auto &render = Get<CRender>();
    render.OnInspector();
    if (render.mModel.mAnimator == nullptr) {
        return;
    }
    ImGui::Text("Animations");
    assert(render.mModel.mAnimator != nullptr);
    int animationId = render.mModel.mAnimator->mCurrentAnimationIndex;
    if (ImGui::SliderInt("Animation Id", &animationId, -1, render.mModel.mAnimator->mAnimations.size() - 1)) {
        render.mModel.mAnimator->PlayAnimation(animationId);
    }
}
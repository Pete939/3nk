
#pragma once
#include <glm/vec3.hpp>

#include "Model.hpp"

class Component {
public:
    Component();
    virtual ~Component() = default;
    virtual void Start() = 0;
    virtual void Update(float dt) = 0;
    virtual void OnInspector() = 0;
    bool mActive = false;
};

class CTransform : public Component {
public:
    glm::vec3 mPosition{0.0f};
    // glm::quat rotation;
    glm::vec3 mScale{1.0f};
    glm::mat4 Matrix() const;
    void Start() override;
    void Update(float dt) override;
    void OnInspector() override;
};

class CAnimation : public Component {

};

class CRender : public Component {
public:
    void Start() override;
    void Update(float dt) override;
    void OnInspector() override;
    Model mModel{};
    bool mVisible = true;
    bool mShowTriangles = false;
    bool mShowBones = false;
    int mSelectedBoneId = 0;
};

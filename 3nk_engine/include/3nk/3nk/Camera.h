#pragma once

#include <glm/glm.hpp>

class Camera {
public:

    Camera();

    /// @param dt Delta time
    void Move(float dt);

    [[nodiscard]] glm::mat4 GetMatrix() const;


private:
    glm::vec3 m_Position;
    glm::vec3 m_Direction;
    glm::vec3 m_Right;
    glm::vec3 m_Up;

    float m_Speed;
    float m_Yaw;
    float m_Pitch;

    float m_LastX;
    float m_LastY;
};

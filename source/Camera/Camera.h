#pragma once

#include <glm/glm.hpp>

class Camera {
public:

    Camera();

    /// @param dt Delta time
    void Move(float dt);

    [[nodiscard]] glm::mat4 GetMatrix() const;

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;

    float speed;
    float yaw;
    float pitch;
};

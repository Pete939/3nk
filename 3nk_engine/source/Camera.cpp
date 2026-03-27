
#include "3nk/Camera.h"

#include <fmt/core.h>
#include <glm/ext/matrix_transform.hpp>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

Camera::Camera() : position(0.0f, 0.0f, 3.0f), direction(0.0f, 0.0f, -1.0f), speed(2.5f),
           yaw(-90.0f), pitch(0.0f) {
    glm::vec3 up_(0.0f, 1.0f, 0.0f);
    right = glm::normalize(glm::cross(up_, direction));
    up = glm::cross(direction, right);
}

void Camera::Move(float dt) {
    const bool *keys = SDL_GetKeyboardState(NULL);
    float speed_ = speed * dt;
    // direction.y = 0; - for FPS style camera.
    if (keys[SDL_SCANCODE_RIGHT] or keys[SDL_SCANCODE_D]) {
        position += glm::normalize(glm::cross(direction, up)) * speed_;
    }
    if (keys[SDL_SCANCODE_LEFT] or keys[SDL_SCANCODE_A]) {
        position -= glm::normalize(glm::cross(direction, up)) * speed_;
    }
    /* Normalization of direction vector here is only necessary if the goal is to make an FPS style camera. */
    if (keys[SDL_SCANCODE_DOWN] or keys[SDL_SCANCODE_S]) {
        position -= direction * speed_;
    }
    if (keys[SDL_SCANCODE_UP] or keys[SDL_SCANCODE_W]) {
        position += direction * speed_;
    }
    if (keys[SDL_SCANCODE_PAGEUP] or keys[SDL_SCANCODE_Q]) {
        position.y += speed_;
    }
    if (keys[SDL_SCANCODE_PAGEDOWN] or keys[SDL_SCANCODE_E]) {
        position.y -= speed_;
    }

    float mouseX;
    float mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    float sensitivity = 0.1f;
    mouseX *= sensitivity;
    mouseY *= sensitivity;
    yaw += mouseX;
    pitch += mouseY;
    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    direction.x = std::cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = -std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);
}

// namespace nk3 {
//     glm::mat4 lookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) {
//         glm::mat4 R(0.0f);
//         glm::vec3 dir = glm::normalize(pos - target);
//         glm::vec3 right = glm::normalize(glm::cross(up, dir));
//         up = glm::cross(dir, right);
//
//         R[0][0] = right.x;
//         R[1][0] = right.y;
//         R[2][0] = right.z;
//         R[0][1] = up.x;
//         R[1][1] = up.y;
//         R[2][1] = up.z;
//         R[0][2] = dir.x;
//         R[1][2] = dir.y;
//         R[2][2] = dir.z;
//         R[3][3] = 1.0f;
//
//         glm::mat4 T(1.0f);
//         T[3][0] = -pos.x;
//         T[3][1] = -pos.y;
//         T[3][2] = -pos.z;
//
//         return R * T;
//     }
// }

glm::mat4 Camera::GetMatrix() const {
    return glm::lookAt(position, position + direction, up);
}


#include "3nk/Camera.h"

#include <fmt/core.h>
#include <glm/ext/matrix_transform.hpp>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

Camera::Camera() : m_Position(0.0f, 0.0f, 3.0f), m_Direction(0.0f, 0.0f, -1.0f), m_Speed(2.5f),
           m_Yaw(-90.0f), m_Pitch(0.0f) {
    glm::vec3 up_(0.0f, 1.0f, 0.0f);
    m_Right = glm::normalize(glm::cross(up_, m_Direction));
    m_Up = glm::cross(m_Direction, m_Right);
}

void Camera::Move(float dt) {
    const bool *keys = SDL_GetKeyboardState(NULL);
    float speed_ = m_Speed * dt;
    // direction.y = 0; - for FPS style camera.
    if (keys[SDL_SCANCODE_RIGHT] or keys[SDL_SCANCODE_D]) {
        m_Position += glm::normalize(glm::cross(m_Direction, m_Up)) * speed_;
    }
    if (keys[SDL_SCANCODE_LEFT] or keys[SDL_SCANCODE_A]) {
        m_Position -= glm::normalize(glm::cross(m_Direction, m_Up)) * speed_;
    }
    /* Normalization of direction vector here is only necessary if the goal is to make an FPS style camera. */
    if (keys[SDL_SCANCODE_DOWN] or keys[SDL_SCANCODE_S]) {
        m_Position -= m_Direction * speed_;
    }
    if (keys[SDL_SCANCODE_UP] or keys[SDL_SCANCODE_W]) {
        m_Position += m_Direction * speed_;
    }
    if (keys[SDL_SCANCODE_PAGEUP] or keys[SDL_SCANCODE_Q]) {
        m_Position.y += speed_;
    }
    if (keys[SDL_SCANCODE_PAGEDOWN] or keys[SDL_SCANCODE_E]) {
        m_Position.y -= speed_;
    }

    float mouseX;
    float mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (keys[SDL_SCANCODE_LCTRL]) {
        float xOffset = mouseX - m_LastX;
        float yOffset = mouseY - m_LastY;
        m_LastX = mouseX;
        m_LastY = mouseY;
        float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;
        m_Yaw += xOffset;
        m_Pitch += yOffset;
        if (m_Pitch > 89.0f) {
            m_Pitch = 89.0f;
        }
        if (m_Pitch < -89.0f) {
            m_Pitch = -89.0f;
        }

        m_Direction.x = std::cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Direction.y = -std::sin(glm::radians(m_Pitch));
        m_Direction.z = std::sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Direction = glm::normalize(m_Direction);
    }
    else {
        m_LastX = mouseX;
        m_LastY = mouseY;
    }

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
    return glm::lookAt(m_Position, m_Position + m_Direction, m_Up);
}

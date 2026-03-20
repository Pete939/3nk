#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <filesystem>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

#include "Camera/Camera.h"
#include "Texture/Texture.h"
#include "Shader/Shader.h"
#include "Model/Importer.h"
#include "Error/glError.hpp"
#include "Model/Model.hpp"

const char *title = "Model loading";
int screenWidth = 800;
int screenHeight = 600;
SDL_Window *window;
SDL_GLContext context;
float proportion = 0.5f;
bool running = true;
bool wireframeMode = false;
uint32_t targetFrameTime = 16; // 16ms ~ 60fps
uint64_t frameStart;
SDL_Event event;
Shader shader;
Camera camera;
Importer importer;
std::vector<Model> models;

void InitWindow();

void ProcessEvents();

void Render();

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    fs::path current_path = fs::current_path();
    std::string path = current_path.string();
    fmt::print("Current path: {}\n", path);

    InitWindow();

    glEnable(GL_DEPTH_TEST);

    shader.createShaderProgram(ASSETS_DIR"shaders/triangle.vert", ASSETS_DIR"shaders/triangle.frag");

    std::vector<std::string> paths = {
        // ASSETS_DIR"models/triangle-sparse/triangle-sparse.gltf",
        // ASSETS_DIR"models/simple-mesh/scene.gltf",
        // ASSETS_DIR"models/triangle/scene.gltf",
        // ASSETS_DIR"models/cube/cube.gltf",
        // ASSETS_DIR"models/cube-interleaved/scene.gltf",
        // ASSETS_DIR"models/suzanne/scene.gltf",
        // ASSETS_DIR"models/avocado/scene.gltf",
        // ASSETS_DIR"models/material-triangle/scene.gltf",
        // ASSETS_DIR"models/simple-texture/scene.gltf",
        ASSETS_DIR"models/cube-textured/scene.gltf",
        ASSETS_DIR"models/cube-textured-non-power-of-two/scene.gltf",
        ASSETS_DIR"models/cube textured with spaces/scene.gltf",
        // ASSETS_DIR"models/duck/scene.gltf",
        // ASSETS_DIR"models/skull/scene.gltf",
        // ASSETS_DIR"models/birthday_cake/scene.gltf",
    };

    models.resize(paths.size());
    for (size_t i = 0; i < models.size(); ++i) {
        importer.LoadJSON(paths[i].c_str(), models[i]);
        models[i].InitOpenGLBuffers();
    }


    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    while (running) {
        float currentFrame = (float) SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessEvents();
        camera.Move(deltaTime);
        Render();

        uint64_t frameEnd = SDL_GetTicks();
        uint32_t frameDuration = frameEnd - frameStart;
        if (frameDuration < targetFrameTime) {
            SDL_Delay(targetFrameTime - frameDuration);
        }
    }
    SDL_Quit();
    return 0;
}

float aspect = (float) screenWidth / (float) screenHeight;

void Render() {
    glClearColor(0.20f, 0.70f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = camera.GetMatrix();
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    float start = -(float)models.size() * 2.0f;
    for (size_t i = 0; i < models.size(); ++i) {
        auto transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(start + (float)i * 2.5f, 0.0f, -2.0f));
        // transform = glm::scale(transform, glm::vec3(10.0f));
        shader.setMat4("model", transform);
        models[i].Render(shader);
    }
    glBindVertexArray(0);
    SDL_GL_SwapWindow(window);
}

void InitWindow() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        fmt::print("Failed to initialize SDL: {}\n", SDL_GetError());
        exit(1);
    }

    unsigned long int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    // flags |= SDL_WINDOW_FULLSCREEN;
    // flags |= SDL_WINDOW_BORDERLESS;
    window = SDL_CreateWindow(
        title,
        screenWidth,
        screenHeight,
        flags);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if (!window) {
        fmt::print("Failed to create window: {}\n", SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        fmt::print("Failed to create opengl context: {}\n", SDL_GetError());
        SDL_Quit();
        exit(3);
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fmt::print("Glad was not initilized.\n");
        exit(4);
    }

    // SDL_SetWindowRelativeMouseMode(window, true);
}

void ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                running = false;
            }
            // Debug
            if (event.key.scancode == SDL_SCANCODE_G) {
                wireframeMode = !wireframeMode;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                if (wireframeMode) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
            }
        }
    }
}

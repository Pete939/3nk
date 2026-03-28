#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <filesystem>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "3nk/AssimpImporter.hpp"
#include "3nk/Camera.h"
#include "3nk/Texture.h"
#include "3nk/Shader.h"
#include "3nk/Importer.h"
#include "3nk/glError.hpp"
#include "3nk/Model.hpp"

const char *title = "3nk engine demo";
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

Shader lightObjectShader;
Shader lightSourceShader;
Camera camera;
Importer importer;
Model suzanne;
Model lightSource;

void InitWindow();

void ProcessEvents();

void Render();

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    fs::path current_path = fs::current_path();
    std::string path = current_path.string();
    fmt::print("Current path: {}\n", path);

    InitWindow();

    // glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    lightObjectShader.createShaderProgram(ASSETS_DIR"shaders/LightAffectedObject.vert", ASSETS_DIR"shaders/LightAffectedObject.frag");
    lightSourceShader.createShaderProgram(ASSETS_DIR"shaders/BasicLightSource.vert", ASSETS_DIR"shaders/BasicLightSource.frag");

    // std::vector<std::string> paths = {
    //     // ASSETS_DIR"models/triangle-sparse/triangle-sparse.gltf",
    //     ASSETS_DIR"models/simple-mesh/scene.gltf",
    //     ASSETS_DIR"models/triangle/scene.gltf",
    //     ASSETS_DIR"models/cube/cube.gltf",
    //     ASSETS_DIR"models/cube-interleaved/scene.gltf",
    //     ASSETS_DIR"models/suzanne/scene.gltf",
    //     ASSETS_DIR"models/avocado/scene.gltf",
    //     ASSETS_DIR"models/material-triangle/scene.gltf",
    //     ASSETS_DIR"models/simple-texture/scene.gltf",
    //     ASSETS_DIR"models/cube-textured/scene.gltf",
    //     ASSETS_DIR"models/cube-vertex-colored/scene.gltf",
    //     ASSETS_DIR"models/cube-non-smoothed-faces/scene.gltf",
    //     ASSETS_DIR"models/cube-textured-non-power-of-two/scene.gltf",
    //     ASSETS_DIR"models/cube textured with spaces/scene.gltf",
    //     // ASSETS_DIR"models/duck/scene.gltf",
    // };

    const char* suzannePath = ASSETS_DIR"models/suzanne/scene.gltf";
    AssimpImport(suzanne, suzannePath);
    suzanne.InitOpenGLBuffers();


    const char* lightSourcePath = ASSETS_DIR"models/cube/cube.gltf";
    importer.LoadJSON(lightSourcePath, lightSource);
    lightSource.InitOpenGLBuffers();


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
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = camera.GetMatrix();
    lightObjectShader.use();
    auto model = glm::mat4(1.0f);
    lightObjectShader.setMat4("view", view);
    lightObjectShader.setMat4("projection", projection);
    lightObjectShader.setVec3("LightColor", glm::vec3(1.0f));
    float time = (float)SDL_GetTicks() / 1000.0f;

    auto transform = glm::mat4(1.0f);
    glm::vec3 suzannePosition = glm::vec3(0.0f, 0.0f, -3.0f);
    model = glm::translate(model, suzannePosition);
    lightObjectShader.setMat4("model", model);
    float radius = 2.0f;
    float x = std::sin(time) * radius;
    float z = std::cos(time) * radius;
    glm::vec3 position = {x, 0.0f, suzannePosition.z + z};
    lightObjectShader.setVec3("LightPos", position);
    suzanne.AssimpRender(lightObjectShader);


    lightSourceShader.use();
    lightSourceShader.setMat4("view", view);
    lightSourceShader.setMat4("projection", projection);
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::scale(transform, glm::vec3(0.2f));
    lightSource.SetTransformMatrix(transform);
    lightSource.Render(lightSourceShader);

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
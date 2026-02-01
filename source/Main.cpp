#include <iostream>
#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <filesystem>
#include <cmath>
#include <glm/glm.hpp>
#include <stb/stb_image.h>
#include "texture/Texture.h"
#include "shader/Shader.h"

const char *title = "OpenGL";
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
uint32_t vao, vbo, ebo;
Shader shader;

void InitWindow();
void ProcessEvents();
void Render();

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    fs::path current_path = fs::current_path();
    std::string path = current_path.string();
    std::cout << fmt::format("Current path: {}\n", path);

    InitWindow();

    float vertices[] = {
        // positions // colors // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left
    };

    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    shader.createShaderProgram(ASSETS_DIR"shaders/triangle.vert", ASSETS_DIR"shaders/triangle.frag");
    shader.use();
    Texture texture0(ASSETS_DIR"textures/container.jpg");
    Texture texture1(ASSETS_DIR"textures/awesomeface.png");
    texture0.use(0);
    texture1.use(1);
    shader.setInt("Texture0", 0);
    shader.setInt("Texture1", 1);


    while (running) {
        frameStart = SDL_GetTicks();
        ProcessEvents();
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

void Render() {
    glClearColor(0.20f, 0.70f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    float offsetX = std::sin(frameStart / 2000.0f) / 2.0f;
    shader.setFloat("offsetX", offsetX);
    float offsetY = std::cos(frameStart / 2000.0f) / 2.0f;
    shader.setFloat("offsetY", offsetY);
    shader.setFloat("proportion", proportion);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    SDL_GL_SwapWindow(window);
}

void InitWindow() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        std::cerr << fmt::format("Failed to initialize SDL: {}\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        title,
        screenWidth,
        screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if (!window) {
        std::cerr << fmt::format("Failed to create window: {}\n", SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cerr << fmt::format("Failed to create opengl context: {}\n", SDL_GetError());
        SDL_Quit();
        exit(3);
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cerr << "Glad was not initilized.\n";
        exit(4);
    }
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
            if (event.key.scancode == SDL_SCANCODE_DOWN) {
                proportion = std::max(0.0f, proportion - 0.02f);
            }
            if (event.key.scancode == SDL_SCANCODE_UP) {
                proportion = std::min(1.0f, proportion + 0.02f);
            }
        }
    }
}

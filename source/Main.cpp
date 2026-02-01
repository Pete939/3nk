#include <iostream>
#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include "shader/Shader.h"
#include <filesystem>
#include <cmath>

const char *title = "OpenGL";
int width = 800;
int height = 600;
SDL_Window *window;
SDL_GLContext context;

void InitWindow();

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    fs::path current_path = fs::current_path();
    std::string path = current_path.string();
    std::cout << fmt::format("Current path: {}\n", path);

    InitWindow();

    float vertices[] = {
        // x     y     z    r     g     b
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    uint32_t indices[] = {
        0, 1, 2,
    };

    Shader triangleShader(ASSETS_DIR"shaders/triangle.vert", ASSETS_DIR"shaders/triangle.frag");

    uint32_t vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bool running = true;
    bool wireframeMode = false;
    uint32_t targetFrameTime = 16; // 16ms ~ 60
    SDL_Event event;
    while (running) {
        uint64_t frameStart = SDL_GetTicks();

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

        glClearColor(0.20f, 0.70f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        triangleShader.use();
        float offsetX = std::sin(frameStart / 2000.0f) / 2.0f;
        triangleShader.setFloat("offsetX", offsetX);
        float offsetY = std::cos(frameStart / 2000.0f) / 2.0f;
        triangleShader.setFloat("offsetY", offsetY);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        SDL_GL_SwapWindow(window);

        uint64_t frameEnd = SDL_GetTicks();
        uint32_t frameDuration = frameEnd - frameStart;
        if (frameDuration < targetFrameTime) {
            SDL_Delay(targetFrameTime - frameDuration);
        }
    }
    SDL_Quit();
    return 0;
}

void InitWindow() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << fmt::format("Failed to initilize SDL: {}\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        title,
        width,
        height,
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
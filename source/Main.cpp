
#include <iostream>
#include <SDL3/SDL.h>
#include "glad/glad.h"

const char *vertexShaderSource = "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "layout (location = 1) in vec3 aCol;\n"
            "out vec4 Col;\n"
            "void main()\n"
            "{\n"
            " gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
            " Col = vec4(aCol, 1.0f);\n"
            "}";

const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec4 Col;\n"
        "void main()\n"
        "{\n"
        " FragColor = Col;\n"
        "}";


int CompileShader(const char *source, GLenum type);
int LinkShader(int vertexShader, int fragmentShader);

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "Failed to initilize SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }

    const char *title = "OpenGL";
    int width = 800;
    int height = 600;
    SDL_Window *window = SDL_CreateWindow(
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
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(2);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cerr << "Failed to create opengl context: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(3);
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cerr << "Glad was not initilized.\n";
        exit(4);
    }

    float vertices[] = {
        // x     y     z    r     g     b
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    uint32_t indices[] = {
        0, 1, 2,
    };

    int vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
    int fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    int shaderProgram = LinkShader(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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
        glUseProgram(shaderProgram);
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

int CompileShader(const char *source, GLenum type) {
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char errorLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char shaderType[16];
        if (type == GL_VERTEX_SHADER) {
            snprintf(shaderType, 16, "VERTEX");
        } else {
            snprintf(shaderType, 16, "FRAGMENT");
        }
        glGetShaderInfoLog(shader, 512, NULL, errorLog);
        return -1;
    }

    return shader;
}

int LinkShader(int vertexShader, int fragmentShader) {
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char errorLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, errorLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n", errorLog);
        return -1;
    }
    return shaderProgram;
}
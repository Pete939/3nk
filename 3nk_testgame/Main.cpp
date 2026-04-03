#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>
#include "3nk/AssimpImporter.hpp"
#include "3nk/Camera.h"
#include "3nk/Shader.h"
#include "3nk/Model.hpp"
#include <3nk/Scene.hpp>

class TestScene : public IScene {
public:
    void Init() override;
    void Run() override;
    void ProcessEvents() override;
    void Render() override;
    void Update() override;
    ~TestScene() override;

private:
    const char *title = "3nk engine demo - skinning";
    int screenWidth = 800;
    int screenHeight = 600;
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
    bool running = true;
    bool wireframeMode = false;
    bool boneRender = false;
    uint32_t targetFrameTime = 16; // 16ms ~ 60fps
    SDL_Event event{};
    float lastFrame = 0;
    float dt = 0;
    int selectedBoneId = 0;
    Shader skinnedModelShader;
    Camera camera;
    Model skinnedModel;
};

int main(int argc, char **argv) {
    {
        TestScene scene;
        scene.Init();
        scene.Run();
    }
    return 0;
}

void TestScene::Init() {
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

    glEnable(GL_DEPTH_TEST);

    std::string vertexShaderPath = ASSETS_DIR"shaders/SkinnedObject.vert";
    std::string fragmentShaderPath= ASSETS_DIR"shaders/SkinnedObjectVisualizeBones.frag";
    skinnedModelShader.createShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    const char* path = ASSETS_DIR"models/cesium-man/scene.gltf";
    AssimpImport(skinnedModel, path);
    skinnedModel.InitOpenGLBuffers();
}

void TestScene::Render() {
    // float time = (float)SDL_GetTicks() / 1000.0f;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = this->screenWidth / this->screenHeight;
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = camera.GetMatrix();
    auto model = glm::mat4(1.0f);
    glm::vec3 position = glm::vec3(0.0f, -0.5f, -1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));

    skinnedModelShader.use();
    skinnedModelShader.setMat4("model", model);
    skinnedModelShader.setMat4("view", view);
    skinnedModelShader.setMat4("projection", projection);
    skinnedModelShader.setInt("selectedBoneId", selectedBoneId);
    skinnedModelShader.setBool("boneRender", boneRender);

    skinnedModel.AssimpRender(skinnedModelShader);
    glBindVertexArray(0);
    SDL_GL_SwapWindow(window);
}

void TestScene::Run() {
    while (this->running) {
        this->Update();
    }
    SDL_DestroyWindow(this->window);
    SDL_GL_DestroyContext(this->context);
}

void TestScene::Update() {
    uint64_t frameStart = SDL_GetTicks();
    float currentFrame = (float)frameStart/ 1000.0f;
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    this->ProcessEvents();
    this->camera.Move(dt);
    this->Render();

    uint64_t frameEnd = SDL_GetTicks();
    uint32_t frameDuration = frameEnd - frameStart;
    if (frameDuration < targetFrameTime) {
        SDL_Delay(targetFrameTime - frameDuration);
    }
}

TestScene::~TestScene() {
    SDL_Quit();
}

void TestScene::ProcessEvents() {
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
            // Switch between bones
            if (event.key.scancode == SDL_SCANCODE_SPACE and boneRender) {
                selectedBoneId = (selectedBoneId + 1) % skinnedModel.numBones;
            }

            if (event.key.scancode == SDL_SCANCODE_B) {
                boneRender = !boneRender;
            }
        }
    }
}

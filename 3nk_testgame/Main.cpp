#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>
#include "3nk/AssimpImporter.hpp"
#include "3nk/Camera.h"
#include "3nk/Shader.h"
#include "3nk/Model.hpp"
#include <3nk/Scene.hpp>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "nfd.h"

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
    int screenWidth = 1920;
    int screenHeight = 1080;
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
    bool running = true;
    bool wireframeMode = false;
    bool boneRender = false;
    bool loadedModel = false;
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

    NFD_Init();

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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // SDL_SetWindowRelativeMouseMode(window, true);

    glEnable(GL_DEPTH_TEST);

    std::string vertexShaderPath = ASSETS_DIR"shaders/SkinnedObject.vert";
    std::string fragmentShaderPath= ASSETS_DIR"shaders/SkinnedObjectVisualizeBones.frag";
    skinnedModelShader.createShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    // const char* path = ASSETS_DIR"models/simple-skin/scene.gltf";
    // const char* path = ASSETS_DIR"models/rigged-simple/scene.gltf";
    const char* path = ASSETS_DIR"models/cesium-man/scene.gltf";
    // const char* path = ASSETS_DIR"models/rigged-figure/scene.gltf";
}

bool my_tool_active = true;
void TestScene::Render() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();


    ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O")) {
                nfdu8char_t *outPath;
                nfdu8filteritem_t filters[1] = { { "Asset files", "gltf,obj,fbx" }};
                nfdopendialogu8args_t args = {0};
                args.filterList = filters;
                args.filterCount = 1;
                nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
                if (result == NFD_OKAY)
                {
                    puts("Success!");
                    puts(outPath);
                    AssimpImport(skinnedModel, outPath);
                    skinnedModel.InitOpenGLBuffers();
                    loadedModel = true;
                    NFD_FreePathU8(outPath);
                }
                else if (result == NFD_CANCEL)
                {
                    puts("User pressed cancel.");
                }
                else
                {
                    printf("Error: %s\n", NFD_GetError());
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */ }
            if (ImGui::MenuItem("Close", "Ctrl+W"))  { my_tool_active = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Edit a color stored as 4 floats

    float my_color[4];
    ImGui::ColorEdit4("Color", my_color);

    // Generate samples and plot them
    float samples[100];
    for (int n = 0; n < 100; n++)
        samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
    ImGui::PlotLines("Samples", samples, 100);

    // Display contents in a scrolling region
    ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
    ImGui::BeginChild("Scrolling");
    for (int n = 0; n < 50; n++)
        ImGui::Text("%04d: Some text", n);
    ImGui::EndChild();
    ImGui::End();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    ImGui::ShowDemoWindow();
    ImGui::Render();

    float r = 0.5f;
    float g = 0.5f;
    float b = 0.5f;
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = this->screenWidth / this->screenHeight;
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = camera.GetMatrix();
    // TODO: Add utility to transform entire model

    if (loadedModel) {
        skinnedModelShader.use();
        skinnedModelShader.setMat4("view", view);
        skinnedModelShader.setMat4("projection", projection);
        skinnedModelShader.setInt("selectedBoneId", selectedBoneId);
        skinnedModelShader.setBool("boneRender", boneRender);

        skinnedModel.AssimpRender(skinnedModelShader, this->dt, false);
    }
    glBindVertexArray(0);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
        ImGui_ImplSDL3_ProcessEvent(&event);
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
                selectedBoneId = (selectedBoneId + 1) % skinnedModel.mAnimator->mCurrentAnimation->mNumBones;
            }

            if (event.key.scancode == SDL_SCANCODE_B) {
                boneRender = !boneRender;
            }
        }
    }
}

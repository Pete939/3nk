#include <fmt/core.h>
#include <SDL3/SDL.h>
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>

#include <3nk/3nk.hpp>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

class TestScene : public IScene {
public:

    void Init() override;

    void Run() override;

    void ProcessEvents() override;

    void Render() override;

    void Update() override;

    ~TestScene() override;

    void RenderUI();

    void InitSkybox();

private:
    const char *title = "3nk engine demo - skinning";
    int screenWidth = 1920;
    int screenHeight = 1080;
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
    bool running = true;
    bool loadedModel = false;
    uint32_t targetFrameTime = 16; // 16ms ~ 60fps
    SDL_Event event{};
    float lastFrame = 0;
    float dt = 0;
    Shader skinnedModelShader{};
    Skybox skybox{};
    Shader skyboxShader{};
    Camera camera{};
    int selectedGameObjectId = -1;
};

int main(int argc, char **argv) {
    {
        TestScene scene;
        scene.Init();
        scene.Run();
    }
    return 0;
}

void TestScene::InitSkybox() {
    const std::array<std::string, 6> filenames = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
    std::array<std::string, 6> filenamesFull;
    for (size_t i = 0; i < 6; ++i) {
        filenamesFull[i] = std::string{ASSETS_DIR} + "/textures/skybox/" + filenames[i];
    }
    skybox.Load(filenamesFull);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    skybox.InitGL();
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;
    // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // SDL_SetWindowRelativeMouseMode(window, true);

    glEnable(GL_DEPTH_TEST);

    std::string vertexShaderPath = ASSETS_DIR"shaders/SkinnedObject.vert";
    std::string fragmentShaderPath = ASSETS_DIR"shaders/SkinnedObjectVisualizeBones.frag";
    skinnedModelShader.createShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    vertexShaderPath = ASSETS_DIR"shaders/Skybox.vert";
    fragmentShaderPath = ASSETS_DIR"shaders/Skybox.frag";
    skyboxShader.createShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    // const char* path = ASSETS_DIR"models/simple-skin/scene.gltf";
    // const char* path = ASSETS_DIR"models/rigged-simple/scene.gltf";
    // const char* path = ASSETS_DIR"models/cesium-man/scene.gltf";
    // const char* path = ASSETS_DIR"models/rigged-figure/scene.gltf";

    InitSkybox();

    auto object = EntityManager::Instance().Add("fox");
    auto &render = object.Add<CRender>();
    const char *path = ASSETS_DIR"models/fox/scene.gltf";
    auto &model = render.mModel;
    AssimpImport(model, path);
    model.InitOpenGLBuffers();
    selectedGameObjectId = 0;
}

void TestScene::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    bool toolActive = true;
    ImGui::Begin("Inspector", &toolActive, ImGuiWindowFlags_MenuBar);

    if (selectedGameObjectId != -1) {
        Entity e = EntityManager::Instance().GetEntities()[selectedGameObjectId];
        e.OnInspector();
    }

    ImGui::End();
    ImGui::Render();
}

void TestScene::Render() {
    RenderUI();

    float r = 0.5f;
    float g = 0.5f;
    float b = 0.5f;
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = this->screenWidth / this->screenHeight;
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = camera.GetMatrix();
    skinnedModelShader.use();
    skinnedModelShader.setMat4("view", view);
    skinnedModelShader.setMat4("projection", projection);



    auto entities = EntityManager::Instance().GetEntities();
    uint32_t numEntities = EntityManager::Instance().GetNumEntities();
    for (size_t i = 0; i < numEntities; ++i) {
        auto object = entities[i];
        assert(object.Has<CRender>());
        if (!object.Has<CRender>()) {
            continue;
        }
        auto &cRender = object.Get<CRender>();
        if (cRender.mVisible == false) {
            continue;
        }
        if (cRender.mShowTriangles) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        skinnedModelShader.setBool("boneRender", cRender.mShowBones);
        if (cRender.mShowBones) {
            skinnedModelShader.setInt("selectedBoneId", cRender.mSelectedBoneId);
        }
        auto matrix = object.Get<CTransform>().Matrix();
        cRender.mModel.AssimpRender(skinnedModelShader, matrix, this->dt);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    auto viewNoTranslation = glm::mat4(glm::mat3(view));
    skybox.Render(skyboxShader, viewNoTranslation, projection);


    glGetError();
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
    float currentFrame = (float) frameStart / 1000.0f;
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
        }
    }
}

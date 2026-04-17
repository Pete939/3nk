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
    const char *m_Title = "3nk engine demo - skinning";
    int m_ScreenWidth = 1920;
    int m_ScreenHeight = 1080;
    SDL_Window *m_Window = nullptr;
    SDL_GLContext m_Context = nullptr;
    bool m_Running = true;
    // bool loadedModel = false;
    uint32_t m_TargetFrameRate = 16; // 16ms ~ 60fps
    SDL_Event m_Event{};
    float m_LastFrame = 0;
    float m_DeltaTime = 0;
    Shader m_SkinningShader{};
    Skybox m_Skybox{};
    Shader m_SkyboxShader{};
    Camera m_Camera{};
    int m_SelectedGameObjectId = -1;
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
    const std::array<std::string, 6> filenames = {
        "right.jpg", "left.jpg", "top.jpg",
        "bottom.jpg", "front.jpg", "back.jpg"
    };
    std::array<std::string, 6> filenamesFull;
    for (size_t i = 0; i < 6; ++i) {
        filenamesFull[i] = std::string{ASSETS_DIR} + "/textures/skybox/" + filenames[i];
    }
    m_Skybox.Load(filenamesFull);
    m_SkyboxShader.Use();
    m_SkyboxShader.SetInt("skybox", 0);
    m_Skybox.InitGL();
}

void TestScene::Init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        fmt::print("Failed to initialize SDL: {}\n", SDL_GetError());
        exit(1);
    }

    unsigned long int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    // flags |= SDL_WINDOW_FULLSCREEN;
    // flags |= SDL_WINDOW_BORDERLESS;
    m_Window = SDL_CreateWindow(
        m_Title,
        m_ScreenWidth,
        m_ScreenHeight,
        flags);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if (!m_Window) {
        fmt::print("Failed to create window: {}\n", SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    m_Context = SDL_GL_CreateContext(m_Window);
    if (m_Context == NULL) {
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
    ImGui_ImplSDL3_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // SDL_SetWindowRelativeMouseMode(window, true);

    glEnable(GL_DEPTH_TEST);

    std::string vertexShaderPath = ASSETS_DIR"shaders/SkinnedObject.vert";
    std::string fragmentShaderPath = ASSETS_DIR"shaders/SkinnedObjectVisualizeBones.frag";
    m_SkinningShader.CreateShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    vertexShaderPath = ASSETS_DIR"shaders/Skybox.vert";
    fragmentShaderPath = ASSETS_DIR"shaders/Skybox.frag";
    m_SkyboxShader.CreateShaderProgram(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

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
    m_SelectedGameObjectId = 0;
}

void TestScene::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    bool toolActive = true;
    ImGui::Begin("Inspector", &toolActive, ImGuiWindowFlags_MenuBar);

    if (m_SelectedGameObjectId != -1) {
        Entity e = EntityManager::Instance().GetEntities()[m_SelectedGameObjectId];
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
    float aspect = this->m_ScreenWidth / this->m_ScreenHeight;
    auto projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    auto view = m_Camera.GetMatrix();
    m_SkinningShader.Use();
    m_SkinningShader.SetMat4("view", view);
    m_SkinningShader.SetMat4("projection", projection);



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
        m_SkinningShader.SetBool("boneRender", cRender.mShowBones);
        if (cRender.mShowBones) {
            m_SkinningShader.SetInt("selectedBoneId", cRender.mSelectedBoneId);
        }
        auto matrix = object.Get<CTransform>().Matrix();
        cRender.mModel.AssimpRender(m_SkinningShader, matrix, this->m_DeltaTime);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    auto viewNoTranslation = glm::mat4(glm::mat3(view));
    m_Skybox.Render(m_SkyboxShader, viewNoTranslation, projection);


    glGetError();
    glBindVertexArray(0);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_Window);
}

void TestScene::Run() {
    while (this->m_Running) {
        this->Update();
    }
    SDL_DestroyWindow(this->m_Window);
    SDL_GL_DestroyContext(this->m_Context);
}

void TestScene::Update() {
    uint64_t frameStart = SDL_GetTicks();
    float currentFrame = (float) frameStart / 1000.0f;
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    this->ProcessEvents();
    this->m_Camera.Move(m_DeltaTime);
    this->Render();

    uint64_t frameEnd = SDL_GetTicks();
    uint32_t frameDuration = frameEnd - frameStart;
    if (frameDuration < m_TargetFrameRate) {
        SDL_Delay(m_TargetFrameRate - frameDuration);
    }
}

TestScene::~TestScene() {
    SDL_Quit();
}

void TestScene::ProcessEvents() {
    while (SDL_PollEvent(&m_Event)) {
        ImGui_ImplSDL3_ProcessEvent(&m_Event);
        if (m_Event.type == SDL_EVENT_QUIT) {
            m_Running = false;
        }
        if (m_Event.type == SDL_EVENT_KEY_DOWN) {
            if (m_Event.key.scancode == SDL_SCANCODE_ESCAPE) {
                m_Running = false;
            }
        }
    }
}

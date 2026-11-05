 <h1 align="center">
  <br>
  <b>3nk</b>
  <br>
  <sub><sup>3nk, a small C++20 OpenGL game engine and rendering demo.</sup></sub>
  <br>
</h1>

## Details

3nk is a small, experimental game engine written in C++. The repository contains the engine library and a test game that demonstrates its rendering and editor features.

- OpenGL 3.3 rendering through GLAD
- SDL3 windowing, input, and OpenGL context management
- glTF and other 3D model importing through Assimp
- Static and skinned model rendering
- Skeletal animation data structures and animation playback support
- Entity and component-based scene organization
- Transform and render components
- Camera and skybox support
- Texture loading through stb_image
- Dear ImGui inspector UI

## Test game

The `3nk_testgame` executable demonstrates the engine with:

- A textured floor and skybox
- A rigged fox model with skeletal animation
- A real-time inspector for selected entities
- Static mesh and skinned mesh shaders
- Resizable SDL3/OpenGL window at 1920x1080 by default

## Project structure

```text
3nk_engine/
├── include/       Engine and third-party headers
└── source/        Engine implementation

3nk_testgame/
├── Main.cpp       Test game entry point
├── assets/        Models, textures, and GLSL shaders
└── include/imgui/ Dear ImGui sources and backends
```

## Dependencies

The project requires:

- CMake 4.1 or newer
- A C++20-compatible compiler
- OpenGL 3.3 or newer
- SDL3
- Assimp
- fmt
- Native File Dialog (nfd)

The dependencies are discovered by CMake using package configuration files. `SDL3`, `assimp`, `fmt`, and `nfd` must be installed and available to CMake.

GLM is required by the engine and should also be available on the compiler's include path.

## Build and run

Clone the repository and configure it with CMake:

```shell
git clone https://github.com/Pete939/3nk.git
cd 3nk
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build
```

Run the test game from the repository root so its asset paths resolve correctly:

```shell
./build/3nk_testgame
```

The build defines `ASSETS_DIR` from the source tree, so the demo loads its models, textures, and shaders from `3nk_testgame/assets`.

## Development notes

This project is an ongoing engine experiment. APIs and systems may change as rendering, scene management, animation, and tooling continue to evolve.

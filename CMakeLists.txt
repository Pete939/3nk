cmake_minimum_required(VERSION 4.1)
project(3nk)
set(CMAKE_CXX_STANDARD 20)

add_library(3nk_engine
        3nk_engine/source/glad.c
        3nk_engine/source/Shader.cpp
        3nk_engine/source/stb_image.c
        3nk_engine/source/Texture.cpp
        3nk_engine/source/Camera.cpp
        3nk_engine/source/Importer.cpp
        3nk_engine/source/Model.cpp
        3nk_engine/source/Mesh.cpp
        3nk_engine/source/Node.cpp
        3nk_engine/source/AssimpImporter.cpp
        3nk_engine/include/3nk/3nk/AssimpToGLM.hpp
        3nk_engine/include/3nk/3nk/Animator.hpp
        3nk_engine/include/3nk/3nk/Animation.hpp
)
target_include_directories(3nk_engine
        PUBLIC 3nk_engine/include/glad
        PUBLIC 3nk_engine/include/stb
        PUBLIC 3nk_engine/include/nlohmann
        PUBLIC 3nk_engine/include/base64
        PUBLIC 3nk_engine/include/3nk
)
find_package(fmt REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(3nk_engine PUBLIC fmt PUBLIC SDL3::SDL3 PUBLIC assimp::assimp)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/3nk_testgame/assets/")
add_compile_definitions(
        ASSETS_DIR="${ASSETS_DIR}"
)
add_executable(3nk_testgame 3nk_testgame/Main.cpp)
target_link_libraries(3nk_testgame PRIVATE 3nk_engine)

#target_link_libraries(OpenGL PRIVATE fmt PRIVATE SDL3::SDL3)
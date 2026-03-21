cmake_minimum_required(VERSION 4.1)
project(OpenGL)

set(CMAKE_CXX_STANDARD 20)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets/")
add_compile_definitions(
        ASSETS_DIR="${ASSETS_DIR}"
)

find_package(fmt REQUIRED)

add_executable(OpenGL source/Main.cpp
        source/glad.c
        source/Shader/Shader.cpp
        source/stb_image.c
        source/Texture/Texture.cpp
        source/Camera/Camera.cpp
        source/Model/Importer.cpp
        source/Model/Model.cpp
        source/Model/Mesh.cpp
        source/Model/Node.cpp
        source/Model/Node.hpp
        source/Model/WhiteTexture.hpp)

target_include_directories(OpenGL
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/glad
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/base64
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/gltf
)


find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(OpenGL PRIVATE fmt PRIVATE SDL3::SDL3)
cmake_minimum_required(VERSION 4.1)
project(OpenGL)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)

add_executable(OpenGL source/Main.cpp
        source/glad.c)

target_include_directories(OpenGL
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/glad)


find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(OpenGL PRIVATE fmt PRIVATE SDL3::SDL3)

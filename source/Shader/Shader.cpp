#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fmt/core.h>
#include <glm/gtc/type_ptr.hpp>


Shader::Shader() : id(-1) {
    std::cout << "Creating unitilized shader.\n";
}

void Shader::createShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
    std::ifstream vertexShaderFile;
    vertexShaderFile.open(vertexShaderPath);
    if (vertexShaderFile.fail()) {
        std::cerr << fmt::format("Failed to open vertex shader file: {}\n", vertexShaderPath);
        return;
    }
    std::ifstream fragmentShaderFile;
    fragmentShaderFile.open(fragmentShaderPath);
    if (fragmentShaderFile.fail()) {
        std::cerr << fmt::format("Failed to open fragment shader file: {}\n", fragmentShaderPath);
        return;
    }
    std::stringstream vertexBuffer, fragmentBuffer;
    vertexBuffer << vertexShaderFile.rdbuf();
    fragmentBuffer << fragmentShaderFile.rdbuf();
    std::string vertexSource = vertexBuffer.str();
    std::string fragmentSource = fragmentBuffer.str();

    int vert = CompileShader(vertexSource.c_str(), GL_VERTEX_SHADER);
    int frag = CompileShader(fragmentSource.c_str(), GL_FRAGMENT_SHADER);
    id = LinkShader(vert, frag);

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::Shader(const char *vertexShaderPath, const char *fragmentShaderPath): id(-1) {
    createShaderProgram(vertexShaderPath, fragmentShaderPath);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, glm::vec3 value) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z);
}

void Shader::use() const {
    glUseProgram(id);
}

int Shader::CompileShader(const char *source, GLenum type) {
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

int Shader::LinkShader(int vertexShader, int fragmentShader) {
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


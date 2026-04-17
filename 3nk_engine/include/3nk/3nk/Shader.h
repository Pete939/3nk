#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
    int m_Id;
public:
    Shader();
    Shader(const char *vertexShaderPath, const char *fragmentShaderPath);
    void CreateShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);
    void Use() const;
    void SetBool(const std::string &name, bool value) const;
    void SetInt(const std::string &name, int value) const;
    void SetFloat(const std::string &name, float value) const;
    void SetMat4(const std::string &name, glm::mat4 value) const;
    void SetMat4s(const std::string &name, int count, glm::mat4 *values);
    void SetVec3(const std::string &name, glm::vec3 value) const;

private:
    static int CompileShader(const char *source, GLenum type);
    static int LinkShader(int vertexShader, int fragmentShader);
};

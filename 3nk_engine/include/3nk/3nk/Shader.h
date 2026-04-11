#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    int id;
    Shader();
    Shader(const char *vertexShaderPath, const char *fragmentShaderPath);
    void createShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);
    void use() const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, glm::mat4 value) const;
    void setVec3(const std::string &name, glm::vec3 value) const;
    void setMat4s(const std::string &name, int count, glm::mat4 *values);

private:
    static int CompileShader(const char *source, GLenum type);
    static int LinkShader(int vertexShader, int fragmentShader);
};

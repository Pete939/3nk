
#include "Model.hpp"
#include "WhiteTexture.hpp"

#include <iostream>
#include <fmt/core.h>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
                // case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
                // case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

Model::Model() {}

void Model::Render(Shader &shader) {
    glCheckError();
    shader.use();
    for (auto & node : nodes) {
        // TODO: Re-enable after completing testing.
        // shader.setMat4("model", node.transform);
        if (node.meshIndex >= 0) {
            auto &mesh = meshes[node.meshIndex];
            if (mesh.materialIndex >= 0) {
                // TODO: Keep n=0?
                textures[mesh.materialIndex].use(0);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, GetWhiteTexture());
            }
            mesh.Render(shader);
        }
    }
}

void Model::InitOpenGLBuffers() {
    for (auto & mesh : meshes) {
        mesh.InitOpenGLBuffers();
        if (mesh.materialIndex >= 0) {
            std::string path = directory + textures[mesh.materialIndex].path;
            textures[mesh.materialIndex].load(path);
        }
    }
}


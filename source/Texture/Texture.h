#pragma once
#include <cstdint>
#include <string>

class Texture {
public:
    uint32_t id;
    std::string path;
    struct {
        int magFilter = GL_LINEAR;
        int minFilter = GL_LINEAR;
        int wrapS = GL_REPEAT;
        int wrapT = GL_REPEAT;
    } sampler;
    Texture(std::string path);
    Texture();
    void load(std::string path);
    void use(int n) const;
};

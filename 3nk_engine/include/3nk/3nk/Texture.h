#pragma once
#include <cstdint>
#include <string>

class Texture {
    uint32_t m_Id;
public:
    std::string mPath;
    struct {
        int magFilter = GL_LINEAR;
        int minFilter = GL_LINEAR;
        int wrapS = GL_REPEAT;
        int wrapT = GL_REPEAT;
    } mSampler;
    Texture(std::string path);
    Texture();
    void Load(std::string path);
    void Use(int n) const;
};

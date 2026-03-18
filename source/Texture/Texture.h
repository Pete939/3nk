#pragma once
#include <cstdint>
#include <string>

class Texture {
public:
    uint32_t id;
    std::string path;
    Texture(std::string path);
    Texture();
    void load(std::string path);
    void use(int n) const;
};

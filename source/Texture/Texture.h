#pragma once
#include <cstdint>
#include <string>

class Texture {
public:
    uint32_t id;
    Texture(const std::string &path);
    void use(int n) const;
};

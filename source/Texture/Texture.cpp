#include <cassert>
#include <iostream>
#include <fmt/core.h>
#include <stb/stb_image.h>
#include <glad/glad.h>
#include "./Texture.h"

void Texture::load(std::string path) {
    this->path = path;
    int width, height, nrChannels;
    // stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(path.c_str(), &width,
        &height, &nrChannels, 0);
    if (!data) {
        std::cerr << fmt::format("Failed to load texture: {}\n", path);
        exit(1);
    }
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    std::string filetype = path.substr(path.length() - 3, 3);
    GLint format = GL_RGB;
    if (filetype == "png") {
        format = GL_RGBA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
        0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    id = texture;
}

Texture::Texture() {};

Texture::Texture(std::string path) {
    load(path);
}

void Texture::use(int n) const {
    assert(n >= 0 && n < 16);
    // glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_2D, id);
}

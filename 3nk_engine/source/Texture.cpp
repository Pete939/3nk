#include <cassert>
#include <iostream>
#include <fmt/core.h>
#include <stb/stb_image.h>
#include <glad/glad.h>
#include "3nk/Texture.h"
#include <cmath>

void Texture::Load(std::string path) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    uint8_t *data = stbi_load(path.c_str(), &width,
        &height, &nrChannels, 0);
    if (!data) {
        assert(0);
        std::cerr << fmt::format("Failed to load texture: {}\n", path);
        exit(1);
    }

    this->mPath = path;

    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mSampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mSampler.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mSampler.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mSampler.magFilter);


    GLint format, internalFormat;
    if (nrChannels == 1) {
        format = GL_RED;
    }
    else if (nrChannels == 3) {
        format = GL_RGB;
    }
    else if (nrChannels == 4) {
        format = GL_RGBA;
    }
    internalFormat = format;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
        0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    m_Id = texture;
}

Texture::Texture() {};

Texture::Texture(std::string path) {
    Load(path);
}

void Texture::Use(int n) const {
    assert(n >= 0 && n < 16);
    glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_2D, m_Id);
}

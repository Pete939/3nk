#pragma once
#include <cstdint>
#include "glad/glad.h"

inline uint8_t GetWhiteTexture()
{
    static unsigned int whiteTex = 0;

    if (whiteTex == 0)
    {
        glGenTextures(1, &whiteTex);
        glBindTexture(GL_TEXTURE_2D, whiteTex);
        unsigned char pixel[] = {255, 255, 255, 255};

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return whiteTex;
}

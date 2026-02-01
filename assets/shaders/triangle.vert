#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexCoords;

uniform float offsetX;
uniform float offsetY;

//out vec4 Col;
out vec2 TexCoords;
void main()
{
    float x = aPos.x + offsetX;
    float y = aPos.y + offsetY;
    gl_Position = vec4(x, y, aPos.z, 1.0);
//    Col = gl_Position;
    TexCoords = aTexCoords;
}
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, TexCoords) * vec4(Color, 1.0f);
}
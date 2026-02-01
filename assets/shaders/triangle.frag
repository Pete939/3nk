#version 330 core
out vec4 FragColor;
//in vec4 Col;
in vec2 TexCoords;

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float proportion;

void main()
{
    FragColor = mix(texture(Texture0, TexCoords),
                    texture(Texture1, TexCoords), proportion);
}
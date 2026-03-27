#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D uTexture;
uniform vec3 LightColor;
uniform vec3 LightPos;


void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * LightColor;

    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * LightColor;

    vec3 result = (ambient + diffuse) * Color;
    FragColor = vec4(result, 1.0f) * texture(uTexture, TexCoords);
}
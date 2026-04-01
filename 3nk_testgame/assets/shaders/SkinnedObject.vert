#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aNormal; // Not used currently for animations.
// GLTF2 specification implies that a vertex can be influenced by max of 4 bones/joints, therefore vec4
layout (location = 4) in ivec4 aBoneIds; // Same as joint
layout (location = 5) in vec4 aWeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 Color;
out vec4 Weights;
flat out ivec4 BoneIds;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aTexCoords;
    Color = aColor;
    BoneIds = aBoneIds;
    Weights = aWeight;
}
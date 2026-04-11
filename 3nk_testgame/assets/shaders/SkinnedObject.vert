#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aNormal; // Not used currently for animations.
// GLTF2 specification implies that a vertex can be influenced by max of 4 bones/joints, therefore vec4
layout (location = 4) in ivec4 aBoneIds; // Same as joint
layout (location = 5) in vec4 aWeight;

uniform mat4 model; // - replaced with bone matrices;
uniform mat4 view;
uniform mat4 projection;

#define MAX_BONES 100

uniform mat4 boneMatrices[MAX_BONES];

void main_better()
{
    // FIXME: Handle negative id's in aBoneIds
}

out vec2 TexCoords;
out vec3 Color;
out vec4 Weights;
flat out ivec4 BoneIds;
void main() {
//    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aTexCoords;
    Color = aColor;
    BoneIds = aBoneIds;
    Weights = aWeight;
    mat4 skinMatrix = mat4(0.0f);
//        aWeight.y * boneMatrices[int(aBoneIds.y)] +
//        aWeight.z * boneMatrices[int(aBoneIds.z)] +
//        aWeight.w * boneMatrices[int(aBoneIds.w)];
    for (int i = 0; i < 4; ++i) {
        if (aBoneIds[i] != -1) {
            skinMatrix += aWeight[i] * boneMatrices[int(aBoneIds[i])];
        }
    }
//    model = mat4(1.0f);
    gl_Position = projection * view * model * skinMatrix * vec4(aPos, 1.0f);
}


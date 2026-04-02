#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;
in vec4 Weights;
flat in ivec4 BoneIds;
uniform sampler2D uTexture;
uniform int selectedBoneId;
uniform bool boneRender = true;

void BoneDebugRender();
void RegularRender();

void main() {
    if (boneRender) {
        BoneDebugRender();
    }
    else {
        RegularRender();
    }
}

void BoneDebugRender() {
    bool found = false;
    for (int i = 0; i < 4; ++i) {
        if (BoneIds[i] == -1) {
            break;
        }
        if (BoneIds[i] == selectedBoneId) {
            if (Weights[i] >= 0.7f) {
            FragColor = vec4(1.0f, 0.0f, 0.0f, 0.0f) * Weights[i];
        }
        else if (Weights[i] >= 0.4f && Weights[i] <= 0.6f) {
            FragColor = vec4(0.0f, 1.0f, 0.0f, 0.0f) * Weights[i];
        }
        else if (Weights[i] >= 0.1f) {
            FragColor = vec4(1.0f, 1.0f, 0.0f, 0.0f) * Weights[i];
        }
            found = true;
            break;
        }
    }
    if (!found) {
        FragColor = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }
}

void RegularRender() {
    FragColor = texture(uTexture, TexCoords) * vec4(Color, 1.0f);
}
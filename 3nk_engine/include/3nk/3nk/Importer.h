#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Model.hpp"

class Importer {
public:
    Importer() = default;
    void LoadJSON(const char *filename, Model &model);
};

namespace gltf {
    struct AccessorSparse {
        int count;

        struct {
            int bufferView;
            int byteOffset = 0;
            int componentType;
            // extensions
            // extras
        } indices;

        struct {
            int bufferView;
            int byteOffset = 0;
            // extensions
            // extras
        } values;

        // extensions
        // extras
    };

    struct Accessor {
        std::optional<int> bufferView;
        int byteOffset = 0;
        int componentType;
        bool normalized = false;
        int count;
        std::string type;
        std::optional<std::vector<float> > max;
        std::optional<std::vector<float> > min;
        std::optional<AccessorSparse> sparse;
        std::optional<std::string> name;
        // Extension extensions
        // Extras extras
    };

    struct Buffer {
        std::vector<uint8_t> uri;
        int byteLength;
        std::optional<std::string> name;
        // extensions
        // extras
    };

    struct BufferView {
        int buffer;
        int byteOffset = 0;
        int byteLength;
        int byteStride = 0;
        std::optional<int> target;
        std::optional<std::string> name;
        // extensions
        // extras
    };

    struct Sampler {
        int magFilter = GL_LINEAR;
        int minFilter = GL_LINEAR;
        int wrapS = GL_REPEAT;
        int wrapT = GL_REPEAT;
        std::optional<std::string> name;
        // extensions
        // extras
    };

    struct Texture {
        // When undefined, a sampler with repeat wrapping and auto filtering SHOULD be used.
        int sampler = -1;
        // When undefined, an extension or other mechanism SHOULD supply an alternate texture source, otherwise behavior is undefined.
        int source = -1;
    };

    struct TextureInfo {
        int index;
        int texCoord = 0;
        // extensions
        // extras
    };

    struct PBRMetallicRoughness {
        std::optional<TextureInfo> baseColorTexture;
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
    };

    struct Material {
        std::optional<std::string> name;
        // extensions
        // extras
        PBRMetallicRoughness pbrMetallicRoughness;
        // normalTexture
        // occlusionTexture
        // emissiveTexture
        float emissiveFactor[3] = {0.0f, 0.0f, 0.0f};
        std::string alphaMode{"OPAQUE"};
        float alphaCutoff = 0.5f;
        bool doubleSided = false;
    };

    struct MeshPrimitive {
        // std::unordered_map<std::string, size_t> attributes;
        struct {
            std::optional<int> position;
            std::optional<int> normal;
            std::optional<int> texCoord; // FIXME: int -> vector<int>?
            std::optional<int> color;
        } attributes;

        std::optional<int> indices;
        std::optional<int> material;
        int mode = 4;
        // targets (optional)
        // extensions
        // extras
    };

    struct Mesh {
        std::vector<MeshPrimitive> primitives;
        // weights (optional)
        std::optional<std::string> name;
        // extensions
        // extras
    };

    struct Image {
        std::optional<std::string> uri;
        std::optional<std::string> mimeType;
        std::optional<int> bufferView;
        std::optional<std::string> name;
        // extensions
        // extras
    };

    struct Node {
        std::optional<int> camera;
        std::optional<std::vector<int> > children;
        std::optional<int> skin;
        glm::mat4 matrix{1.0f};
        std::optional<int> mesh;
        float rotation[4] = {0, 0, 0, 1};
        float scale[3] = {0, 0, 0};
        float translation[3] = {0, 0, 0};
        // weights (optional)
        std::optional<std::string> name;
        // extensions
        // extras
    };


    enum MeshPrimitiveModes {
        POINTS = 0,
        LINES = 1,
        LINE_LOOP = 2,
        LINE_STRIP = 3,
        TRIANGLES = 4,
        TRIANGLES_STRIP = 5,
        TRIANGLE_FAN = 6
    };
}

struct ImportCtx {
    std::vector<gltf::Accessor> accessors;
    std::vector<gltf::Buffer> buffers;
    std::vector<gltf::BufferView> bufferViews;
    std::vector<gltf::Image> images;
    std::vector<gltf::Material> materials;
    std::vector<gltf::Mesh> meshes;
    std::vector<gltf::Sampler> samplers;
    std::vector<gltf::Texture> textures;
};
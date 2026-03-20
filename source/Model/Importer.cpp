/* If a member of a struct/class is of type std::optional<T> it means that according to
 * the glt2 specification it is not a required property and does NOT have a default value.
 * If a member is not required but has a default value, then this value should be specified in the
 * struct/class definition for that member.
*/

#include "Importer.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <fmt/core.h>
#include <map>
#include <vector>
#include "base64/base64.hpp"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "stb/stb_image.h"

// #include <iostream>

#define ARRAY_BUFFER 34962
#define ELEMENT_ARRAY_BUFFER 34963

std::string decodeUri(const std::string &uri) {
    std::string result;
    // result.reserve(uri.length());
    for (size_t i = 0; i < uri.length(); ++i) {
        if (uri[i] == '%' and i + 2 < uri.length()) {
            auto code = uri.substr(i + 1, 2);
            char c = std::stoi(code, nullptr, 16);
            result += c;
            i += 2;
        }
        else {
            result += uri[i];
        }
    }
    return result;
}

using json = nlohmann::json;
uint32_t loadTexture(const std::string &path) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(path.c_str(), &width,
        &height, &nrChannels, 0);
    if (!data) {
        fmt::print("Failed to load texture: {}\n", path);
        exit(1);
    }
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    std::string filetype = path.substr(path.length() - 3, 3);
    GLint format = GL_RGB;
    if (filetype == "png") {
        format = GL_RGBA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
        0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return texture;
}

// TODO: Basic Texture support
// TODO: byteStride support
void Importer::LoadJSON(const char *filename, Model &model) {
    std::ifstream file(filename);
    if (!file.good()) {
        fmt::print("Failed to load {}!\n", filename);
        exit(1);
    }
    json input;
    file >> input;

    assert(input["asset"]["version"] == "2.0");

    std::string directory(filename);
    size_t dir = directory.rfind('/');
    model.directory = directory.substr(0, dir + 1);

    // Load to structures
    std::unordered_map<std::string, size_t> counts;
    counts["scene"] = input["scenes"].size();
    counts["node"] = input["nodes"].size();
    counts["mesh"] = input["meshes"].size();
    counts["bufferView"] = input["bufferViews"].size();
    counts["buffer"] = input["buffers"].size();
    counts["accessor"] = input["accessors"].size();
    counts["material"] = input["materials"].size();
    counts["texture"] = input["textures"].size();
    counts["image"] = input["images"].size();
    counts["sampler"] = input["samplers"].size();
    for (const std::pair<const std::string, size_t> &count: counts) {
        fmt::print("{} count: {}\n", count.first, count.second);
    }

    // std::vector<gltf::Node> nodes{counts["node"]};
    model.nodes.resize(counts["node"]);
    for (size_t i = 0; i < model.nodes.size(); ++i) {
        auto &jnode = input["nodes"][i];
        auto &node = model.nodes[i];

        if (jnode.contains("children")) {
            node.childrenIndices.resize(jnode["children"].size());
            for (size_t j = 0; j < node.childrenIndices.size(); ++j) {
                node.childrenIndices[j] = jnode["children"][j];
            }
        }

        if (jnode.contains("matrix")) {
            glm::mat4 matrix(0.0f);
            for (size_t j = 0; j < 4; ++j) {
                for (size_t k = 0; k < 4; ++k) {
                    matrix[j][k] = jnode["matrix"][j * 4 + k];
                }
            }
        }

        if (jnode.contains("mesh")) {
            node.meshIndex = jnode["mesh"];
        }
        if (jnode.contains("translation")) {
            auto t = jnode["translation"];
            node.transform = glm::translate(node.transform, glm::vec3(t[0], t[1], t[2]));
        }
        // TODO: I think that rotation works correctly but I'm not sure
        if (jnode.contains("rotation")) {
            auto r = jnode["rotation"];
            // (x,y,z,w), where the w-component is the cosine of half of the rotation angle
            float angle = acos((double) r[3]) * 2.0f;
            glm::vec3 axis = glm::vec3(r[0], r[1], r[2]);
            if (axis.x + axis.y + axis.z == 0.0f) {
                angle = 0.0f;
                axis.z = 1.0f;
            }
            node.transform = glm::rotate(node.transform, glm::radians(angle), axis);
        }
        if (jnode.contains("scale")) {
            auto s = jnode["scale"];
            node.transform = glm::scale(node.transform, glm::vec3(s[0], s[1], s[2]));
        }
    }

    model.meshes.resize(counts["mesh"]);
    std::vector<gltf::Mesh> meshes{counts["mesh"]};
    for (size_t i = 0; i < meshes.size(); ++i) {
        auto &jsonMeshPrimitive = input["meshes"][i]["primitives"];
        size_t primitivesCount = jsonMeshPrimitive.size();
        meshes[i].primitives.resize(primitivesCount);
        for (auto &primitive: meshes[i].primitives) {
            if (jsonMeshPrimitive[i].contains("indices")) {
                int indices = jsonMeshPrimitive[i]["indices"];
                primitive.indices = indices;
            }
            if (jsonMeshPrimitive[i].contains("material")) {
                int material = jsonMeshPrimitive[i]["material"];
                primitive.material = material;
            }

            if (jsonMeshPrimitive[i]["attributes"].contains("NORMAL")) {
                int normal = jsonMeshPrimitive[i]["attributes"]["NORMAL"];
                primitive.attributes.normal = normal;
            }
            if (jsonMeshPrimitive[i]["attributes"].contains("POSITION")) {
                int position = jsonMeshPrimitive[i]["attributes"]["POSITION"];
                primitive.attributes.position = position;
            }
            // FIXME: Support more texcoords (0-N)
            if (jsonMeshPrimitive[i]["attributes"].contains("TEXCOORD_0")) {
                int texCoord = jsonMeshPrimitive[i]["attributes"]["TEXCOORD_0"];
                primitive.attributes.texCoord = texCoord;
            }
        }
    }

    std::vector<gltf::Material> materials;
    materials.resize(counts["material"]);
    for (size_t i = 0; i < materials.size(); ++i) {
        auto &material = materials[i];
        auto &jsonMaterial = input["materials"][i];
        if (jsonMaterial.contains("pbrMetallicRoughness")) {
            if (jsonMaterial["pbrMetallicRoughness"].contains("baseColorTexture")) {
                gltf::TextureInfo info;
                info.index =  jsonMaterial["pbrMetallicRoughness"]["baseColorTexture"]["index"];
                material.pbrMetallicRoughness.baseColorTexture.emplace(info);
            }
        }
    }

    std::vector<gltf::Texture> textures;
    textures.resize(counts["texture"]);
    for (size_t i = 0; i < textures.size(); ++i) {
        auto &texture = textures[i];
        auto &jsonTexture = input["textures"][i];
        if (jsonTexture.contains("sampler")) {
            texture.sampler = jsonTexture["sampler"];
        }
        // else {
        //     fmt::print(stderr, "No undefined sampler support for texture!\n");
        //     exit(4);
        // }
        if (jsonTexture.contains("source")) {
            texture.source = jsonTexture["source"];
        }
        else {
            fmt::print(stderr, "No undefined source support for texture!\n");
            exit(5);
        }
    }

    std::vector<gltf::Sampler> samplers;
    samplers.resize(counts["sampler"]);
    for (size_t i = 0; i < samplers.size(); ++i) {
        auto &sampler = samplers[i];
        auto &jsonImage = input["samplers"][i];
        if (jsonImage.contains("magFilter")) {
            sampler.magFilter = jsonImage["magFilter"];
        }
        if (jsonImage.contains("minFilter")) {
            sampler.minFilter = jsonImage["minFilter"];
        }
        if (jsonImage.contains("wrapS")) {
            sampler.wrapS = jsonImage["wrapS"];
        }
        if (jsonImage.contains("wrapT")) {
            sampler.wrapT = jsonImage["wrapT"];
        }
    }

    // TODO: Load images
    std::vector<gltf::Image> images;
    images.resize(counts["image"]);
    for (size_t i = 0; i < images.size(); ++i) {
        auto &image = images[i];
        auto &jsonImage = input["images"][i];
        if (jsonImage.contains("uri")) {
            image.uri = decodeUri(jsonImage["uri"]);
        }
        else {
            fmt::print(stderr, "No bufferView support for image!\n");
            exit(3);
        }
    }

    model.textures.resize(images.size());
    for (size_t i = 0; i < model.textures.size(); ++i) {
        auto &texture = model.textures[i];
        texture.path = images[i].uri.value();
        if (textures[i].sampler >= 0) {
            texture.sampler.minFilter = samplers[textures[i].sampler].minFilter;
            texture.sampler.magFilter = samplers[textures[i].sampler].magFilter;
            texture.sampler.wrapS = samplers[textures[i].sampler].wrapS;
            texture.sampler.wrapT = samplers[textures[i].sampler].wrapT;
            // fmt::print("minFilter: {}\n", texture.sampler.minFilter);
            // fmt::print("magFilter: {}\n", texture.sampler.magFilter);
            // fmt::print("wrapS: {}\n", texture.sampler.wrapS);
            // fmt::print("wrapT: {}\n", texture.sampler.wrapT);
        }
    }


    std::vector<gltf::Accessor> accessors{counts["accessor"]};
    for (size_t i = 0; i < accessors.size(); ++i) {
        auto &accessor = accessors[i];
        auto &jsonAccessor = input["accessors"][i];
        accessor.bufferView = jsonAccessor["bufferView"];
        if (jsonAccessor.contains("byteOffset")) {
            accessor.byteOffset = jsonAccessor["byteOffset"];
        }
        accessor.componentType = jsonAccessor["componentType"];
        accessor.count = jsonAccessor["count"];
        accessor.type = jsonAccessor["type"];

        if (jsonAccessor.contains("max") and jsonAccessor.contains("min")) {
            accessor.max.emplace(std::vector<float>());
            accessor.min.emplace(std::vector<float>());
            size_t minmaxSize = jsonAccessor["max"].size();
            auto &amax = accessor.max.value();
            auto &amin = accessor.min.value();
            amax.resize(minmaxSize);
            amin.resize(minmaxSize);
            for (size_t j = 0; j < minmaxSize; ++j) {
                amax[j] = jsonAccessor["max"][j];
                amin[j] = jsonAccessor["min"][j];
            }
        }

        if (jsonAccessor.contains("sparse")) {
            auto &jsonSparse = jsonAccessor["sparse"];
            accessor.sparse.emplace(gltf::AccessorSparse{});
            auto &sparse = accessor.sparse.value();

            sparse.count = jsonSparse["count"];

            sparse.indices.bufferView = jsonSparse["indices"]["bufferView"];
            sparse.indices.componentType = jsonSparse["indices"]["componentType"];
            if (jsonSparse["indices"].contains("byteOffset")) {
                sparse.indices.byteOffset = jsonSparse["indices"]["byteOffset"];
            }

            sparse.values.bufferView = jsonSparse["values"]["bufferView"];
            if (jsonSparse["values"].contains("byteOffset")) {
                sparse.indices.byteOffset = jsonSparse["values"]["byteOffset"];
            }
        }
    }

    std::vector<gltf::BufferView> bufferViews{counts["bufferView"]};
    for (size_t i = 0; i < bufferViews.size(); ++i) {
        auto &bufferView = bufferViews[i];
        auto &jsonBufferView = input["bufferViews"][i];
        bufferView.buffer = jsonBufferView["buffer"];
        bufferView.byteOffset = jsonBufferView.value("byteOffset", 0);
        bufferView.byteLength = jsonBufferView["byteLength"];
        if (jsonBufferView.contains("byteStride")) {
            bufferView.byteStride = jsonBufferView["byteStride"];
        }
    }

    std::vector<gltf::Buffer> buffers{counts["buffer"]};
    for (size_t i = 0; i < buffers.size(); ++i) {
        auto &buffer = buffers[i];
        auto &jsonBuffer = input["buffers"][i];
        std::string uri = jsonBuffer["uri"];
        int comma = uri.find(',');
        if (comma < 0) {
            std::string path = model.directory + uri;
            fmt::print("{}\n", path);
            std::ifstream binFile(path, std::ios::binary);
            buffer.byteLength = jsonBuffer["byteLength"];
            buffer.uri.resize(buffer.byteLength);
            binFile.read((char*)buffer.uri.data(), buffer.uri.size());
            continue;
        }
        std::string base64 = uri.substr(comma + 1);
        buffer.byteLength = jsonBuffer["byteLength"];
        auto bytes = base64::from_base64(base64);
        // NOTE: Assume uri has value.
        buffer.uri.resize(bytes.length());
        // Load raw bytes
        for (size_t j = 0; j < bytes.length(); ++j) {
            buffer.uri[j] = (uint8_t) bytes[j];
        }
    }

    // Decide which accessor is for positions and which for indices (Mesh?)
    for (size_t i = 0; i < meshes.size(); ++i) {
        auto &mesh = meshes[i];
        std::vector<int> accessors_;
        // NOTE: Very efficient code 🤓
        for (auto &primitive: mesh.primitives) {
            if (primitive.indices.has_value()) {
                accessors_.push_back(primitive.indices.value());
            }
            auto &attr = primitive.attributes;
            if (attr.position.has_value()) {
                accessors_.push_back(attr.position.value());
            }
            if (attr.normal.has_value()) {
                accessors_.push_back(attr.normal.value());
            }
            if (attr.texCoord.has_value()) {
                accessors_.push_back(attr.texCoord.value());
            }
        }
        for (int accIdx : accessors_) {
            auto &accessor = accessors[accIdx];
            auto &bufferView = bufferViews[accessor.bufferView.value()];
            auto &buffer = buffers[bufferView.buffer];
            // Indices
            if (accessor.componentType == GL_UNSIGNED_SHORT && accessor.type == "SCALAR") {
                // GL_UNSIGNED_SHORT
                size_t count = accessor.count;
                model.meshes[i].indices.resize(count);
                auto ptr = (uint16_t *) (buffer.uri.data() + bufferView.byteOffset + accessor.byteOffset);
                for (size_t j = 0; j < count; ++j) {
                    model.meshes[i].indices[j] = ptr[j];
                }
            }
            // Skip normals for now
            if (mesh.primitives[0].attributes.normal.has_value()) {
                if (mesh.primitives[0].attributes.normal.value() == accIdx) {
                    continue;
                }
            }
            // Vertices
            if (accessor.componentType == GL_FLOAT && accessor.type == "VEC3") {
                // fmt::print("POSITIONS\n");
                size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(float) * 3;
                size_t jmp = stride / sizeof(float);
                model.meshes[i].vertices.resize(accessor.count * 3);
                auto ptr = (float*)(buffer.uri.data() + bufferView.byteOffset + accessor.byteOffset);
                for (size_t j = 0; j < accessor.count; ++j) {
                    size_t src = j * jmp;
                    size_t dst = j * 3;
                    model.meshes[i].vertices[dst + 0] = ptr[src + 0];
                    model.meshes[i].vertices[dst + 1] = ptr[src + 1];
                    model.meshes[i].vertices[dst + 2] = ptr[src + 2];
                    // fmt::print("{}, {}, {}\n", ptr[src], ptr[src+1], ptr[src+2]);
                }
            }
            // Texture coordinates
            else if (accessor.componentType == GL_FLOAT && accessor.type == "VEC2") {
                // fmt::print("UVS\n");
                size_t jmp = bufferView.byteStride / sizeof(float);
                std::vector<float> texCoords;
                texCoords.resize(accessor.count * 2);
                // buffer.uri.data() is correct.
                auto ptr = (float *) (buffer.uri.data() + bufferView.byteOffset + accessor.byteOffset);
                for (size_t i = 0; i < accessor.count; ++i) {
                    size_t src = i * jmp;
                    size_t dst = i * 2;
                    texCoords[dst + 0] = ptr[src + 0];
                    texCoords[dst + 1] = ptr[src + 1];
                    // fmt::print("{}, {}\n", ptr[src], ptr[src+1]);
                }
                model.meshes[i].texCoords = std::move(texCoords);
            }

            if (accessor.sparse.has_value()) {
                auto &sparse = accessor.sparse.value();
                auto &idBufV = bufferViews[sparse.indices.bufferView];
                auto &vtBufV = bufferViews[sparse.values.bufferView];

                auto idPtr = (uint16_t *) (buffer.uri.data() + idBufV.byteOffset);
                size_t count = idBufV.byteLength / sizeof(uint16_t);
                std::vector<uint16_t> indices_;
                indices_.resize(count);
                for (size_t j = 0; j < count; ++j) {
                    indices_[j] = idPtr[j];
                }

                auto vtPtr = (float *) (buffer.uri.data() + vtBufV.byteOffset);
                count = vtBufV.byteLength / sizeof(float);
                std::vector<float> values;
                values.resize(count);
                for (size_t j = 0; j < count; ++j) {
                    values[j] = vtPtr[j];
                }

                for (size_t j = 0; j < indices_.size(); ++j) {
                    size_t index = indices_[j];
                    for (size_t k = 0; k < 3; ++k) {
                        model.meshes[i].vertices[index * 3 + k] = values[j * 3 + k];
                    }
                }
            }
        }
        // Materials are processed differrently
        // FIXME: Currently only one primitive set is assumed
        if (mesh.primitives[0].material.has_value()) {
            int material = mesh.primitives[0].material.value();
            if (materials[material].pbrMetallicRoughness.baseColorTexture.has_value()) {
                int texture = materials[material].pbrMetallicRoughness.baseColorTexture.value().index;
                int source = textures[texture].source;
                model.meshes[i].materialIndex = source;
            }
        }
    }
}

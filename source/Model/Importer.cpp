
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

template <typename T>
void LoadModelAttribute(ImportCtx &ctx,std::vector<T> &meshAttr, uint32_t accessorIndex, uint32_t amountPerDataType) {
    auto &accessor = ctx.accessors[accessorIndex];
    auto &bufferView = ctx.bufferViews[accessor.bufferView.value()];
    auto &buffer = ctx.buffers[bufferView.buffer];
    size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(T) * amountPerDataType;
    size_t jmp = stride / sizeof(T);
    meshAttr.resize(accessor.count * amountPerDataType);
    auto ptr = (T*)(buffer.uri.data() + bufferView.byteOffset + accessor.byteOffset);
    for (size_t i = 0; i < accessor.count; ++i) {
        size_t src = i * jmp;
        size_t dst = i * amountPerDataType;
        for (size_t j = 0; j < amountPerDataType; ++j) {
            meshAttr[dst + j] = ptr[src + j];
        }
    }
}

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

    ImportCtx ctx{};

    // std::vector<gltf::Node> nodes{counts["node"]};
    model.nodes.resize(counts["node"]);
    for (size_t i = 0; i < model.nodes.size(); ++i) {
        auto &jnode = input["nodes"][i];
        auto &node = model.nodes[i];

        if (jnode.contains("children")) {
            node.childrenIndices.resize(jnode["children"].size());
            for (size_t j = 0; j < node.childrenIndices.size(); ++j) {
                uint32_t childIndex =  jnode["children"][j];
                node.childrenIndices[j] = childIndex;
                model.nodes[childIndex].parentIndex = i;
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
    ctx.meshes.resize(counts["mesh"]);
    for (size_t i = 0; i < ctx.meshes.size(); ++i) {
        auto &jsonMeshPrimitive = input["meshes"][i]["primitives"];
        size_t primitivesCount = jsonMeshPrimitive.size();
        ctx.meshes[i].primitives.resize(primitivesCount);
        for (size_t j = 0; j < ctx.meshes[i].primitives.size(); ++j) {
            auto &primitive = ctx.meshes[i].primitives[j];
            if (jsonMeshPrimitive[j].contains("indices")) {
                int indices = jsonMeshPrimitive[j]["indices"];
                primitive.indices = indices;
            }
            if (jsonMeshPrimitive[j].contains("material")) {
                int material = jsonMeshPrimitive[j]["material"];
                primitive.material = material;
            }

            if (jsonMeshPrimitive[j]["attributes"].contains("NORMAL")) {
                int normal = jsonMeshPrimitive[j]["attributes"]["NORMAL"];
                primitive.attributes.normal = normal;
            }
            if (jsonMeshPrimitive[j]["attributes"].contains("POSITION")) {
                int position = jsonMeshPrimitive[j]["attributes"]["POSITION"];
                primitive.attributes.position = position;
            }
            // FIXME: Support more texcoords (0-N)
            if (jsonMeshPrimitive[j]["attributes"].contains("TEXCOORD_0")) {
                int texCoord = jsonMeshPrimitive[j]["attributes"]["TEXCOORD_0"];
                primitive.attributes.texCoord = texCoord;
            }

            if (jsonMeshPrimitive[j]["attributes"].contains("COLOR_0")) {
                int color = jsonMeshPrimitive[j]["attributes"]["COLOR_0"];
                primitive.attributes.color = color;
            }
        }
    }

    ctx.materials.resize(counts["material"]);
    for (size_t i = 0; i < ctx.materials.size(); ++i) {
        auto &material = ctx.materials[i];
        auto &jsonMaterial = input["materials"][i];
        if (jsonMaterial.contains("pbrMetallicRoughness")) {
            auto &jsonMR = jsonMaterial["pbrMetallicRoughness"];
            if (jsonMR.contains("baseColorTexture")) {
                gltf::TextureInfo info;
                info.index =  jsonMR["baseColorTexture"]["index"];
                material.pbrMetallicRoughness.baseColorTexture = info;
            }
        }
    }

    ctx.textures.resize(counts["texture"]);
    for (size_t i = 0; i < ctx.textures.size(); ++i) {
        auto &texture = ctx.textures[i];
        auto &jsonTexture = input["textures"][i];
        if (jsonTexture.contains("sampler")) {
            texture.sampler = jsonTexture["sampler"];
        }
        if (jsonTexture.contains("source")) {
            texture.source = jsonTexture["source"];
        }
        else {
            fmt::print(stderr, "No undefined source support for texture!\n");
            exit(5);
        }
    }

    ctx.samplers.resize(counts["sampler"]);
    for (size_t i = 0; i < ctx.samplers.size(); ++i) {
        auto &sampler = ctx.samplers[i];
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

    ctx.images.resize(counts["image"]);
    for (size_t i = 0; i < ctx.images.size(); ++i) {
        auto &image = ctx.images[i];
        auto &jsonImage = input["images"][i];
        if (jsonImage.contains("uri")) {
            image.uri = decodeUri(jsonImage["uri"]);
        }
        else {
            fmt::print(stderr, "No bufferView support for image!\n");
            exit(3);
        }
    }

    model.textures.resize(ctx.images.size());
    for (size_t i = 0; i < model.textures.size(); ++i) {
        auto &texture = model.textures[i];
        texture.path = ctx.images[i].uri.value();
        if (ctx.textures[i].sampler >= 0) {
            auto sampler = ctx.samplers[ctx.textures[i].sampler];
            texture.sampler.minFilter = sampler.minFilter;
            texture.sampler.magFilter = sampler.magFilter;
            texture.sampler.wrapS = sampler.wrapS;
            texture.sampler.wrapT = sampler.wrapT;
        }
    }

    ctx.accessors.resize(counts["accessor"]);
    for (size_t i = 0; i < ctx.accessors.size(); ++i) {
        auto &accessor = ctx.accessors[i];
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

    ctx.bufferViews.resize(counts["bufferView"]);
    for (size_t i = 0; i < ctx.bufferViews.size(); ++i) {
        auto &bufferView = ctx.bufferViews[i];
        auto &jsonBufferView = input["bufferViews"][i];
        bufferView.buffer = jsonBufferView["buffer"];
        bufferView.byteOffset = jsonBufferView.value("byteOffset", 0);
        bufferView.byteLength = jsonBufferView["byteLength"];
        if (jsonBufferView.contains("byteStride")) {
            bufferView.byteStride = jsonBufferView["byteStride"];
        }
    }

    ctx.buffers.resize(counts["buffer"]);
    for (size_t i = 0; i < ctx.buffers.size(); ++i) {
        auto &buffer = ctx.buffers[i];
        auto &jsonBuffer = input["buffers"][i];
        std::string uri = jsonBuffer["uri"];
        int comma = uri.find(',');
        if (comma < 0) {
            std::string path = model.directory + uri;
            // fmt::print("{}\n", path);
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

    for (size_t i = 0; i < ctx.meshes.size(); ++i) {
        auto &mesh = ctx.meshes[i];
        std::vector<int> accessors_;
        for (auto &primitive: mesh.primitives) {
            if (primitive.indices.has_value()) {
                int idx = primitive.indices.value();
                LoadModelAttribute<uint16_t>(ctx, model.meshes[i].indices, idx, 1);
            }
            auto &attr = primitive.attributes;
            if (attr.position.has_value()) {
                int idx = attr.position.value();
                LoadModelAttribute<float>(ctx, model.meshes[i].vertices, idx, 3);
            }
            if (attr.normal.has_value()) {
            }
            if (attr.texCoord.has_value()) {
                int idx = attr.texCoord.value();
                LoadModelAttribute<float>(ctx, model.meshes[i].texCoords, idx, 2);
            }
            if (attr.color.has_value()) {
                int idx = attr.color.value();
                LoadModelAttribute<float>(ctx, model.meshes[i].colors, idx, 2);
            }
        }
        for (int accIdx : accessors_) {
            auto &accessor = ctx.accessors[accIdx];
            auto &bufferView = ctx.bufferViews[accessor.bufferView.value()];
            auto &buffer = ctx.buffers[bufferView.buffer];

            if (accessor.sparse.has_value()) {
                auto &sparse = accessor.sparse.value();
                auto &idBufV = ctx.bufferViews[sparse.indices.bufferView];
                auto &vtBufV = ctx.bufferViews[sparse.values.bufferView];

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
            auto baseColor = ctx.materials[material].pbrMetallicRoughness.baseColorTexture;
            if ((baseColor.has_value())) {
                int texture = baseColor.value().index;
                int source = ctx.textures[texture].source;
                model.meshes[i].materialIndex = source;
            }
        }
    }
}

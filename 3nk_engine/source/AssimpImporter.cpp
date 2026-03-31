#include <3nk/AssimpImporter.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags
#include <assimp/material.h>     // Post-processing flags
#include <fmt/core.h>

void CountNodes(aiNode* node, uint32_t *count) {
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        aiNode *child = node->mChildren[i];
        *count += child->mNumChildren;
        CountNodes(child, count);
    }
}

bool IsTextureAlreadyLoaded(const Model &model, const std::string &path) {
    for (auto &texture : model.textures) {
        if (!texture.path.empty()) {
            if (std::strcmp(texture.path.c_str(), path.c_str()) == 0) {
                return true;
            }
        }
    }
    return false;
}

void ProcessMesh(Mesh &pMesh, aiMesh *pAiMesh, const aiScene *scene, Model &model) {
    if (pAiMesh->HasPositions()) {
        constexpr size_t VERTEX_COMPONENT_COUNT = 3;
        // Process Vertices
        pMesh.vertices.resize(pAiMesh->mNumVertices * VERTEX_COMPONENT_COUNT);
        for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
            aiVector3D vertex = pAiMesh->mVertices[j];
            pMesh.vertices[j * 3 + 0] = vertex.x;
            pMesh.vertices[j * 3 + 1] = vertex.y;
            pMesh.vertices[j * 3 + 2] = vertex.z;
        }
        if (pAiMesh->HasTextureCoords(0)) {
            constexpr size_t TEXCOORD_COMPONENT_COUNT = 2;
            pMesh.texCoords.resize(pAiMesh->mNumVertices * TEXCOORD_COMPONENT_COUNT);
            for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
                aiVector3D texCoord = pAiMesh->mTextureCoords[0][j];
                pMesh.texCoords[j * 2 + 0] = texCoord.x;
                pMesh.texCoords[j * 2 + 1] = texCoord.y;
            }
        }
    }
    else {
        fmt::print(stderr, "Epic import fail. Model has no vertices lol\n");
    }

    if (pAiMesh->HasFaces()) {
        // Assume always triangles.
        constexpr size_t INDICES_IN_EACH_FACE = 3;
        pMesh.indices.resize(pAiMesh->mNumFaces * INDICES_IN_EACH_FACE);
        for (size_t j = 0; j < pAiMesh->mNumFaces; ++j) {
            aiFace face = pAiMesh->mFaces[j];
            assert(face.mNumIndices == INDICES_IN_EACH_FACE);
            pMesh.indices[j * 3 + 0] = face.mIndices[0];
            pMesh.indices[j * 3 + 1] = face.mIndices[1];
            pMesh.indices[j * 3 + 2] = face.mIndices[2];
        }
    }
    else {
        fmt::print(stderr, "Epic import fail. Model has no faces lol\n");
    }

    if (pAiMesh->HasNormals()) {
        constexpr int NORMAL_COMPONENT_COUNT = 3;
        pMesh.normals.resize(pAiMesh->mNumVertices * NORMAL_COMPONENT_COUNT);
        for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
            aiVector3D normal = pAiMesh->mNormals[j];
            pMesh.normals[j * 3 + 0] = normal.x;
            pMesh.normals[j * 3 + 1] = normal.y;
            pMesh.normals[j * 3 + 2] = normal.z;
        }
    }

    // Assume always material to be present?
    if (pAiMesh->mMaterialIndex >= 0) {
        aiMaterial *mat = scene->mMaterials[pAiMesh->mMaterialIndex];
        aiString str;
        mat->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
        std::string path = model.directory + std::string(str.C_Str());
        if (!IsTextureAlreadyLoaded(model, path)) {
            model.textures[0].load(path);
            fmt::print("texture path: {}\n", str.C_Str());
        }
        pMesh.materialIndex = pAiMesh->mMaterialIndex;
        // constexpr int ASSIMP_MAX_TEXTURE_TYPES = 27;
        // fmt::print("TEXTURE TYPE COUNTS\n");
        // for (size_t i = 0; i <= ASSIMP_MAX_TEXTURE_TYPES; ++i) {
            // fmt::print("type {} count: {}\n", i, mat->GetTextureCount(aiTextureType(i)));
        // }
    }
}

void ProcessNode(uint32_t currentNodeIndex, aiNode *pAiNode, Model &model) {

    assert(model.nodes.size() > currentNodeIndex);

    auto &modelNode = model.nodes[currentNodeIndex];

    for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
        uint32_t meshIdx = pAiNode->mMeshes[i];
        modelNode.meshIndex = meshIdx;
    }

    // TODO: Set transform matrix
    // modelNode.transform = pAiNode->mTransformation;

    // Process node's children
    if (pAiNode->mNumChildren > 0) {
        modelNode.childrenIndices.resize(pAiNode->mNumChildren);
        for (size_t i = 0; i < pAiNode->mNumChildren; ++i) {
            modelNode.childrenIndices[i] = ++currentNodeIndex;
            ProcessNode(currentNodeIndex, pAiNode->mChildren[i], model);
        }
    }

}

// TODO: Support textures
void AssimpImport(Model &model, const std::string &filename) {
    // Create an instance of the Importer class
    auto importer = Assimp::Importer{};
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene *scene = importer.ReadFile(filename.c_str(),
                                        aiProcess_CalcTangentSpace |
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene) {
        fmt::print("Failed to load {}: {}\n", filename, importer.GetErrorString());
        exit(69);
    }


    std::string directory(filename);
    size_t dir = directory.rfind('/');
    model.directory = directory.substr(0, dir + 1);

    model.meshes.resize(scene->mNumMeshes);
    model.textures.resize(scene->mNumMaterials);

    // Node process
    uint32_t count = 1 + scene->mRootNode->mNumChildren; // Count root node and it's children
    CountNodes(scene->mRootNode, &count);
    fmt::print("NODE COUNT: {}\n", count);
    model.nodes.resize(count);
    ProcessNode(0, scene->mRootNode, model);

    // Mesh process
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[i];
        auto &modelMesh = model.meshes[i];
        ProcessMesh(modelMesh, mesh, scene, model);
    }

    if (scene->mNumMaterials > 0) {
        for (size_t i = 0; i < scene->mNumMaterials; ++i) {
            aiMaterial *mat = scene->mMaterials[i];
            aiString name;
            mat->Get(AI_MATKEY_NAME,name);
        }
    }
}
#include <unordered_map>
#include <3nk/AssimpImporter.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post-processing flags
#include <assimp/material.h>     // Post-processing flags
#include <fmt/core.h>

#include <3nk/AssimpToGLM.hpp>

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

    ;
void ProcessMesh(Mesh &pMesh, aiMesh *pAiMesh, const aiScene *scene, Model &model, std::unordered_map<std::string, int> &boneMap) {
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
        if (mat->GetTextureCount(aiTextureType_BASE_COLOR) != 0) {
            std::string path = model.directory + std::string(str.C_Str());
            if (!IsTextureAlreadyLoaded(model, path)) {
                model.textures[0].load(path);
                fmt::print("texture path: {}\n", str.C_Str());
            }
        }

        pMesh.materialIndex = pAiMesh->mMaterialIndex;
        // constexpr int ASSIMP_MAX_TEXTURE_TYPES = 27;
        // fmt::print("TEXTURE TYPE COUNTS\n");
        // for (size_t i = 0; i <= ASSIMP_MAX_TEXTURE_TYPES; ++i) {
            // fmt::print("type {} count: {}\n", i, mat->GetTextureCount(aiTextureType(i)));
        // }
    }

    if (pAiMesh->HasBones()) {
        std::vector<int> vertexBoneCount; // Temporary vector that stores how many bones have been discovered per vertex
        vertexBoneCount.resize(pAiMesh->mNumVertices, 0);
        pMesh.vertexToBones.resize(pAiMesh->mNumVertices);
        fmt::print("\t\tmNumBones: {}\n", pAiMesh->mNumBones);
        fmt::print("\t\tBones:\n");
        for (size_t i = 0; i < pAiMesh->mNumBones; ++i) {
            aiBone *bone = pAiMesh->mBones[i];
            fmt::print("\t\t\tbone {}: {}\n", i, bone->mName.C_Str());
            // fmt::print("\t\t\t\tname: {}\n", bone->mName.C_Str());
            BoneInfo bi{};

            bi.mOffsetMatrix = AssimpToGLM::ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
            model.boneInfos.push_back(bi);


            std::string boneName(bone->mName.C_Str());

            if (!boneMap.contains(boneName)) {
                boneMap[boneName] = model.numBones;
                model.numBones++;
            }
            else {
                fmt::print(stderr, "some facing repeat i dont know\n");
                assert(0);
            }
            // if (!bone->mName.Empty()) {
                // fmt::print("Bone {} name: {}\n", i, bone->mName.data);
            // }
            for (size_t j = 0; j < bone->mNumWeights; ++j) {
                aiVertexWeight weight = bone->mWeights[j];
                uint32_t idx = vertexBoneCount[weight.mVertexId]++;
                pMesh.vertexToBones[weight.mVertexId].boneIds[idx] = i;
                pMesh.vertexToBones[weight.mVertexId].weights[idx] = weight.mWeight;
                // fmt::print("\tWeight {}: vertexId: {}, Weight: {}\n", j, weight.mVertexId, weight.mWeight);
            }
        }
    }
}

void ProcessNode(uint32_t currentNodeIndex, aiNode *pAiNode, Model &model, int *pCurrentIndex) {

    currentNodeIndex = *pCurrentIndex;
    assert(model.nodes.size() > currentNodeIndex);
    auto &modelNode = model.nodes[currentNodeIndex];

    assert(pAiNode->mName.C_Str() != nullptr);
    modelNode.name = std::string(pAiNode->mName.C_Str());
    fmt::print("\tnode {}: {}\n", currentNodeIndex, modelNode.name);

    assert(pAiNode->mNumMeshes <= 1);
    for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
        uint32_t meshIdx = pAiNode->mMeshes[i];
        modelNode.meshIndex = meshIdx;
        fmt::print("\t\tmesh index: {}\n", meshIdx);
    }

    modelNode.transform = AssimpToGLM::ConvertMatrixToGLMFormat(pAiNode->mTransformation);

    // Process node's children
    int nodeIndex = currentNodeIndex;
    if (pAiNode->mNumChildren > 0) {
        modelNode.childrenIndices.resize(pAiNode->mNumChildren);
        for (size_t i = 0; i < pAiNode->mNumChildren; ++i) {
            modelNode.childrenIndices[i] = ++(*pCurrentIndex);
            ProcessNode(currentNodeIndex, pAiNode->mChildren[i], model, pCurrentIndex);
        }
        // fmt::print("\tnode {} child indices:", nodeIndex);
        // for (size_t i = 0; i < pAiNode->mNumChildren; ++i) {
        //     fmt::print(" {}", modelNode.childrenIndices[i]);
        // }
        // fmt::print("\n");
    }

}

void ProcessAnimations(Model &model, const aiScene *scene) {
    fmt::print("mNumAnimations: {}\n", scene->mNumAnimations);
    uint32_t animationNodesCount = 0;
    std::unordered_map<std::string, int> animationMap;
    assert(scene->mNumAnimations == 1);
    for (size_t i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation *animation = scene->mAnimations[i];
        fmt::print("DURATION IN TICKS: {}\n", animation->mDuration);
        fmt::print("TICKS PER SECOND: {}\n", animation->mTicksPerSecond);
        model.animDuration = animation->mDuration / animation->mTicksPerSecond;
        fmt::print("mNumChannels {}\n", animation->mNumChannels);
        model.animationNodes.resize(animation->mNumChannels);
        for (int j = 0; j < animation->mNumChannels; ++j) {
            aiNodeAnim *nodeAnim = animation->mChannels[j];

            std::string animNodeName(nodeAnim->mNodeName.C_Str());
            animationMap[animNodeName] = animationNodesCount++;

            fmt::print("animNode {}: {}\n", animationNodesCount - 1, nodeAnim->mNodeName.C_Str());
            model.animationNodes[j].positionKeys.resize(nodeAnim->mNumPositionKeys);
            // fmt::print("\tmNumPositionKeys: {}\n", nodeAnim->mNumPositionKeys);
            for (size_t k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
                aiVectorKey key = nodeAnim->mPositionKeys[k];
                model.animationNodes[j].positionKeys[k].position.x = key.mValue.x;
                model.animationNodes[j].positionKeys[k].position.y = key.mValue.y;
                model.animationNodes[j].positionKeys[k].position.z = key.mValue.z;
                model.animationNodes[j].positionKeys[k].timeStamp = key.mTime;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
            // fmt::print("\tmNumScalingKeys {}\n", nodeAnim->mNumScalingKeys);
            model.animationNodes[j].scalingKeys.resize(nodeAnim->mNumScalingKeys);
            for (size_t k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
                aiVectorKey key = nodeAnim->mScalingKeys[k];
                model.animationNodes[j].scalingKeys[k].scale.x = key.mValue.x;
                model.animationNodes[j].scalingKeys[k].scale.y = key.mValue.y;
                model.animationNodes[j].scalingKeys[k].scale.z = key.mValue.z;
                model.animationNodes[j].scalingKeys[k].timeStamp = key.mTime;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
            // fmt::print("\tmNumRotationKeys: {}\n", nodeAnim->mNumRotationKeys);
            model.animationNodes[j].rotationKeys.resize(nodeAnim->mNumRotationKeys);
            for (size_t k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
                aiQuatKey key = nodeAnim->mRotationKeys[k];
                model.animationNodes[j].rotationKeys[k].rotation = AssimpToGLM::GetGLMQuat(key.mValue);
                model.animationNodes[j].rotationKeys[k].timeStamp = key.mTime;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
        }
    }

    for (auto &node: model.nodes) {
        if (!node.name.empty() and animationMap.contains(node.name)) {
            node.animNodeIndex = animationMap[node.name];
        }
    }
}

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
    fmt::print("Node hierarchy\n");
    int index = 0;
    ProcessNode(0, scene->mRootNode, model, &index);

    // Count bones
    int totalBones = 0;
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[i];
        totalBones += mesh->mNumBones;
    }
    model.boneInfos.reserve(totalBones);

    std::unordered_map<std::string, int> boneMap;

    // Mesh process
    fmt::print("Meshes\n");
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        fmt::print("\tMesh {}\n", i);
        aiMesh *mesh = scene->mMeshes[i];
        auto &modelMesh = model.meshes[i];
        ProcessMesh(modelMesh, mesh, scene, model, boneMap);
    }

    // TODO: Link nodes to bones
    for (auto &node: model.nodes) {
        if (!node.name.empty() and boneMap.contains(node.name)) {
            node.boneIndex = boneMap[node.name];
        }
    }

    // assert(model.boneInfos.capacity() == 2);

    model.globalInverseTransform = glm::inverse(model.nodes[0].transform);

    if (scene->HasAnimations()) {
        fmt::print("Animations\n");
        ProcessAnimations(model, scene);
    }

    int amount = 0;
    for (auto &node : model.nodes) {
        fmt::print("node {}: {}\n", amount++, node.animNodeIndex);
    }
}
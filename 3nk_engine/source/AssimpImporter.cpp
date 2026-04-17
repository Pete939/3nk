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
    for (auto &texture : model.mTextures) {
        if (!texture.mPath.empty()) {
            if (std::strcmp(texture.mPath.c_str(), path.c_str()) == 0) {
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
        pMesh.mVertices.resize(pAiMesh->mNumVertices * VERTEX_COMPONENT_COUNT);
        for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
            aiVector3D vertex = pAiMesh->mVertices[j];
            pMesh.mVertices[j * 3 + 0] = vertex.x;
            pMesh.mVertices[j * 3 + 1] = vertex.y;
            pMesh.mVertices[j * 3 + 2] = vertex.z;
        }
        if (pAiMesh->HasTextureCoords(0)) {
            constexpr size_t TEXCOORD_COMPONENT_COUNT = 2;
            pMesh.mTexCoords.resize(pAiMesh->mNumVertices * TEXCOORD_COMPONENT_COUNT);
            for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
                aiVector3D texCoord = pAiMesh->mTextureCoords[0][j];
                pMesh.mTexCoords[j * 2 + 0] = texCoord.x;
                pMesh.mTexCoords[j * 2 + 1] = texCoord.y;
            }
        }
    }
    else {
        fmt::print(stderr, "Epic import fail. Model has no vertices lol\n");
    }

    if (pAiMesh->HasFaces()) {
        // Assume always triangles.
        constexpr size_t INDICES_IN_EACH_FACE = 3;
        pMesh.mIndices.resize(pAiMesh->mNumFaces * INDICES_IN_EACH_FACE);
        for (size_t j = 0; j < pAiMesh->mNumFaces; ++j) {
            aiFace face = pAiMesh->mFaces[j];
            assert(face.mNumIndices == INDICES_IN_EACH_FACE);
            pMesh.mIndices[j * 3 + 0] = face.mIndices[0];
            pMesh.mIndices[j * 3 + 1] = face.mIndices[1];
            pMesh.mIndices[j * 3 + 2] = face.mIndices[2];
        }
    }
    else {
        fmt::print(stderr, "Epic import fail. Model has no faces lol\n");
    }

    if (pAiMesh->HasNormals()) {
        constexpr int NORMAL_COMPONENT_COUNT = 3;
        pMesh.mNormals.resize(pAiMesh->mNumVertices * NORMAL_COMPONENT_COUNT);
        for (size_t j = 0; j < pAiMesh->mNumVertices; ++j) {
            aiVector3D normal = pAiMesh->mNormals[j];
            pMesh.mNormals[j * 3 + 0] = normal.x;
            pMesh.mNormals[j * 3 + 1] = normal.y;
            pMesh.mNormals[j * 3 + 2] = normal.z;
        }
    }

    // Assume always material to be present?
    if (pAiMesh->mMaterialIndex >= 0) {
        aiMaterial *mat = scene->mMaterials[pAiMesh->mMaterialIndex];
        aiString str;
        mat->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
        if (mat->GetTextureCount(aiTextureType_BASE_COLOR) != 0) {
            std::string path = model.mDirectory + std::string(str.C_Str());
            if (!IsTextureAlreadyLoaded(model, path)) {
                model.mTextures[0].Load(path);
                fmt::print("texture path: {}\n", str.C_Str());
            }
        }

        pMesh.mMaterialIndex = pAiMesh->mMaterialIndex;
        // constexpr int ASSIMP_MAX_TEXTURE_TYPES = 27;
        // fmt::print("TEXTURE TYPE COUNTS\n");
        // for (size_t i = 0; i <= ASSIMP_MAX_TEXTURE_TYPES; ++i) {
            // fmt::print("type {} count: {}\n", i, mat->GetTextureCount(aiTextureType(i)));
        // }
    }

    if (pAiMesh->HasBones()) {
        std::vector<int> vertexBoneCount; // Temporary vector that stores how many bones have been discovered per vertex
        vertexBoneCount.resize(pAiMesh->mNumVertices, 0);
        pMesh.mVertexToBones.resize(pAiMesh->mNumVertices);
        fmt::print("\t\tmNumBones: {}\n", pAiMesh->mNumBones);
        fmt::print("\t\tBones:\n");
        for (size_t i = 0; i < pAiMesh->mNumBones; ++i) {
            aiBone *bone = pAiMesh->mBones[i];
            fmt::print("\t\t\tbone {}: {}\n", i, bone->mName.C_Str());
            // fmt::print("\t\t\t\tname: {}\n", bone->mName.C_Str());
            BoneInfo bi{};

            bi.offsetMatrix = AssimpToGLM::ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
            // model.boneInfos.push_back(bi);
            model.mAnimator->mBoneInfos.push_back(bi);


            std::string boneName(bone->mName.C_Str());

            // Kinda bs hack but will suffice for now
            int32_t &numBones = model.mAnimator->mNumBones;

            if (!boneMap.contains(boneName)) {
                boneMap[boneName] = numBones;
                numBones++;
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
                pMesh.mVertexToBones[weight.mVertexId].boneIds[idx] = i;
                pMesh.mVertexToBones[weight.mVertexId].weights[idx] = weight.mWeight;
                // fmt::print("\tWeight {}: vertexId: {}, Weight: {}\n", j, weight.mVertexId, weight.mWeight);
            }
        }
    }
}

void ProcessNode(uint32_t currentNodeIndex, aiNode *pAiNode, Model &model, int *pCurrentIndex) {

    currentNodeIndex = *pCurrentIndex;
    assert(model.mNodes.size() > currentNodeIndex);
    auto &modelNode = model.mNodes[currentNodeIndex];

    assert(pAiNode->mName.C_Str() != nullptr);
    modelNode.mName = std::string(pAiNode->mName.C_Str());
    fmt::print("\tnode {}: {}\n", currentNodeIndex, modelNode.mName);

    assert(pAiNode->mNumMeshes <= 1);
    for (size_t i = 0; i < pAiNode->mNumMeshes; ++i) {
        uint32_t meshIdx = pAiNode->mMeshes[i];
        modelNode.mMeshIndex = meshIdx;
        fmt::print("\t\tmesh index: {}\n", meshIdx);
    }

    modelNode.mTransform = AssimpToGLM::ConvertMatrixToGLMFormat(pAiNode->mTransformation);

    // Process node's children
    int nodeIndex = currentNodeIndex;
    if (pAiNode->mNumChildren > 0) {
        modelNode.mChildrenIndices.resize(pAiNode->mNumChildren);
        for (size_t i = 0; i < pAiNode->mNumChildren; ++i) {
            modelNode.mChildrenIndices[i] = ++(*pCurrentIndex);
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
    std::unordered_map<std::string, int> animationMap;
    // assert(scene->mNumAnimations == 1);
    for (size_t i = 0; i < scene->mNumAnimations; ++i) {
        uint32_t animationNodesCount = 0;
        aiAnimation *animation = scene->mAnimations[i];
        auto &ANIMATION = model.mAnimator->mAnimations[i];
        fmt::print("DURATION IN TICKS: {}\n", animation->mDuration);
        fmt::print("TICKS PER SECOND: {}\n", animation->mTicksPerSecond);
        ANIMATION.mDuration = animation->mDuration / animation->mTicksPerSecond;
        fmt::print("mNumChannels {}\n", animation->mNumChannels);
        ANIMATION.mNodes.resize(animation->mNumChannels);
        for (int j = 0; j < animation->mNumChannels; ++j) {
            aiNodeAnim *nodeAnim = animation->mChannels[j];

            std::string animNodeName(nodeAnim->mNodeName.C_Str());
            animationMap[animNodeName] = animationNodesCount++;

            fmt::print("animNode {}: {}\n", animationNodesCount - 1, nodeAnim->mNodeName.C_Str());
            auto &animNode = ANIMATION.mNodes[j];
            animNode.positionKeys.resize(nodeAnim->mNumPositionKeys);
            // fmt::print("\tmNumPositionKeys: {}\n", nodeAnim->mNumPositionKeys);
            for (size_t k = 0; k < nodeAnim->mNumPositionKeys; ++k) {
                aiVectorKey key = nodeAnim->mPositionKeys[k];
                animNode.positionKeys[k].position.x = key.mValue.x;
                animNode.positionKeys[k].position.y = key.mValue.y;
                animNode.positionKeys[k].position.z = key.mValue.z;
                // TODO: Keep time in ticks or millisecond?
                animNode.positionKeys[k].timeStamp = key.mTime / 1000.0f;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
            // fmt::print("\tmNumScalingKeys {}\n", nodeAnim->mNumScalingKeys);
            animNode.scalingKeys.resize(nodeAnim->mNumScalingKeys);
            for (size_t k = 0; k < nodeAnim->mNumScalingKeys; ++k) {
                aiVectorKey key = nodeAnim->mScalingKeys[k];
                animNode.scalingKeys[k].scale.x = key.mValue.x;
                animNode.scalingKeys[k].scale.y = key.mValue.y;
                animNode.scalingKeys[k].scale.z = key.mValue.z;
                animNode.scalingKeys[k].timeStamp = key.mTime / 1000.0f;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
            // fmt::print("\tmNumRotationKeys: {}\n", nodeAnim->mNumRotationKeys);
            animNode.rotationKeys.resize(nodeAnim->mNumRotationKeys);
            for (size_t k = 0; k < nodeAnim->mNumRotationKeys; ++k) {
                aiQuatKey key = nodeAnim->mRotationKeys[k];
                animNode.rotationKeys[k].rotation = AssimpToGLM::GetGLMQuat(key.mValue);
                animNode.rotationKeys[k].timeStamp = key.mTime / 1000.0f;
                // fmt::print("\t\tpositionKey {} time: {}\n", k, key.mTime);
            }
        }
        for (auto &node: model.mNodes) {
            if (!node.mName.empty() and animationMap.contains(node.mName)) {
                node.mAnimNodeIndex = animationMap[node.mName];
            }
        }
        animationMap.clear();
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
    model.mDirectory = directory.substr(0, dir + 1);

    model.mMeshes.resize(scene->mNumMeshes);
    model.mTextures.resize(scene->mNumMaterials);

    // Node process
    uint32_t count = 1 + scene->mRootNode->mNumChildren; // Count root node and it's children
    CountNodes(scene->mRootNode, &count);
    fmt::print("NODE COUNT: {}\n", count);
    model.mNodes.resize(count);
    fmt::print("Node hierarchy\n");
    int index = 0;
    ProcessNode(0, scene->mRootNode, model, &index);

    bool hasAnimations = false;
    model.mAnimator = new Animator;
    model.mAnimator->mAnimations.resize(scene->mNumAnimations);
    assert(model.mAnimator != nullptr);

    int totalBones = 0;
    if (scene->HasAnimations()) {
        hasAnimations = true;
        fmt::print("Animations\n");
        ProcessAnimations(model, scene);
        // Count bones
        for (size_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh *mesh = scene->mMeshes[i];
            totalBones += mesh->mNumBones;
        }
    }

    assert(hasAnimations == true);


    model.mAnimator->mBoneInfos.reserve(totalBones);
    // model.boneInfos.reserve(totalBones);

    std::unordered_map<std::string, int> boneMap;

    // Mesh process
    fmt::print("Meshes\n");
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        fmt::print("\tMesh {}\n", i);
        aiMesh *mesh = scene->mMeshes[i];
        auto &modelMesh = model.mMeshes[i];
        ProcessMesh(modelMesh, mesh, scene, model, boneMap);
    }

    // TODO: Link nodes to bones
    for (auto &node: model.mNodes) {
        if (!node.mName.empty() and boneMap.contains(node.mName)) {
            node.mBoneIndex = boneMap[node.mName];
        }
    }

    // assert(model.boneInfos.capacity() == 2);

    // model.globalInverseTransform = glm::inverse(model.nodes[0].transform);

    model.mAnimator->mModelNodes = &model.mNodes;


    int amount = 0;
    for (auto &node : model.mNodes) {
        fmt::print("node {}: {}\n", amount++, node.mAnimNodeIndex);
    }
}

#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <string>
#include <vector>

class Node {
public:
    int meshIndex = -1;
    int parentIndex = -1;
    int boneIndex = -1;
    int animNodeIndex = -1;
    glm::mat4 transform{1.0f};
    std::vector<uint32_t> childrenIndices;
    std::string name;
};

// https://github.khronos.org/glTF-Tutorials/gltfTutorial/gltfTutorial_004_ScenesNodes.html
// Global transforms of nodes
//
// Regardless of the representation in the JSON file, the local transform of a node can be stored as a 4×4 matrix. The global transform of a node is given by the product of all local transforms on the path from the root to the respective node:
//
// Structure:           local transform      global transform
// root                 R                    R
//  +- nodeA            A                    R*A
//      +- nodeB        B                    R*A*B
//      +- nodeC        C                    R*A*C
//
// It is important to point out that after the file was loaded these global transforms can not be computed only once.
// Later, it will be shown how animations may modify the local transforms of individual nodes.
// And these modifications will affect the global transforms of all descendant nodes.
// Therefore, when the global transform of a node is required, it has to be computed directly from the current local transforms of all nodes.
// Alternatively, and as a potential performance improvement, an implementation could cache the global transforms,
// detect changes in the local transforms of ancestor nodes, and update the global transforms only when necessary.
// The different implementation options for this will depend on the programming language and the requirements for the client application,
// and thus are beyond the scope of this tutorial.

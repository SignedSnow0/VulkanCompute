#pragma once

#include <glm/glm.hpp>

#include "Core/AssetManager.h"

struct Triangle {
    glm::vec3 V0;
    glm::vec3 V1;
    glm::vec3 V2;
};

struct BvhNode {
    glm::vec3 Min{ FLT_MAX };
    glm::vec3 Max{ -FLT_MAX };
    uint32_t ChildIndex{ 0 };
    uint32_t TriangleIndex{ 0 };
    uint32_t TriangleCount{ 0 };
};

class BvhBuilder {
public:
    BvhBuilder(const std::shared_ptr<Mesh>& mesh, uint32_t maxDepth);

    std::vector<BvhNode> Build();

private:
    void BuildLayer(BvhNode& parent, uint32_t depth);
    
    std::shared_ptr<Mesh> mMesh;
    std::vector<BvhNode> mBvh;
    std::vector<Triangle> mTriangles;

    uint32_t mMaxDepth;
};
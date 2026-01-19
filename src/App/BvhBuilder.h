#pragma once

#include <glm/glm.hpp>

#include "Core/AssetManager.h"

struct Triangle {
    alignas(16) glm::vec3 V0;
    alignas(16) glm::vec3 V1;
    alignas(16) glm::vec3 V2;
};

struct BvhNode {
    alignas(16) glm::vec3 Min{ FLT_MAX };
    alignas(16) glm::vec3 Max{ -FLT_MAX };
    uint32_t ChildIndex{ 0 };
    uint32_t TriangleIndex{ 0 };
    uint32_t TriangleCount{ 0 };
};

class BvhBuilder {
public:
    BvhBuilder(const std::shared_ptr<Mesh>& mesh, uint32_t maxDepth);

    void Build(bool printStats = true);

    [[nodiscard]] const std::vector<BvhNode>& GetBvh() const {
        return mBvh;
    }

    [[nodiscard]] const std::vector<Triangle>& GetTriangles() const {
        return mTriangles;
    }

private:
    void BuildLayer(BvhNode& parent, uint32_t depth);
    void PrintStats();

    std::shared_ptr<Mesh> mMesh;
    std::vector<BvhNode> mBvh;
    std::vector<Triangle> mTriangles;

    uint32_t mMaxDepth;
};
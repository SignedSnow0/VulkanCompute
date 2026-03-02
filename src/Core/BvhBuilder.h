#pragma once

#include <glm/glm.hpp>

#include "Core/AssetManager.h"
#include "Core/Model.h"

struct BvhNode {
    alignas(16) glm::vec3 Min{ FLT_MAX };
    alignas(16) glm::vec3 Max{ -FLT_MAX };
    uint32_t ChildIndex{ 0 };
    uint32_t TriangleIndex{ 0 };
    uint32_t TriangleCount{ 0 };
};

class BvhBuilder {
public:
    BvhBuilder(const Mesh& mesh, uint32_t maxDepth);

    void Build(bool printStats = true);

    [[nodiscard]] const std::vector<BvhNode>& GetBvh() const {
        return mBvh;
    }

    [[nodiscard]] const std::vector<Triangle>& GetTriangles() const {
        return mTriangles;
    }

    void ExportToCSV(const std::string& filepath) const;

private:
    void BuildLayer(BvhNode& parent, uint32_t depth);
    void PrintStats();

    const Mesh& mMesh;
    std::vector<BvhNode> mBvh;
    std::vector<Triangle> mTriangles;

    uint32_t mMaxDepth;
};
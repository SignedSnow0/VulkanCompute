#include "BvhBuilder.h"

enum class SplitAxis {
    X,
    Y,
    Z
};

void Grow(BvhNode& node, const Triangle& triangle) {
    node.Min = glm::min(node.Min, glm::min(triangle.V0, glm::min(triangle.V1, triangle.V2)));
    node.Max = glm::max(node.Max, glm::max(triangle.V0, glm::max(triangle.V1, triangle.V2)));
}

bool IsLeft(const Triangle& triangle, SplitAxis axis, float splitPos) {
    float centre = (triangle.V0[static_cast<int>(axis)] +
        triangle.V1[static_cast<int>(axis)] +
        triangle.V2[static_cast<int>(axis)]) / 3.0f;

    return centre < splitPos;
}

void GetLongestAxis(const BvhNode& node, SplitAxis& axis, float& pos) {
    glm::vec3 extents = node.Max - node.Min;
    if (extents.x >= extents.y && extents.x >= extents.z) {
        axis = SplitAxis::X;
        pos = (node.Min.x + node.Max.x) / 2.0f;
    } else if (extents.y >= extents.x && extents.y >= extents.z) {
        axis = SplitAxis::Y;
        pos = (node.Min.y + node.Max.y) / 2.0f;
    } else {
        axis = SplitAxis::Z;
        pos = (node.Min.z + node.Max.z) / 2.0f;
    }
}

std::vector<Triangle> ExtractTriangles(const std::shared_ptr<Mesh>& mesh) {
    std::vector<Triangle> triangles;
    const auto& vertices = mesh->GetVertices();
    const auto& indices = mesh->GetIndices();

    for (size_t i = 0; i < indices.size(); i += 3) {
        Triangle tri;
        tri.V0 = vertices[indices[i]].position;
        tri.V1 = vertices[indices[i + 1]].position;
        tri.V2 = vertices[indices[i + 2]].position;
        triangles.push_back(tri);
    }

    return triangles;
}

BvhBuilder::BvhBuilder(const std::shared_ptr<Mesh>& mesh, uint32_t maxDepth)
    : mMesh(mesh), mMaxDepth(maxDepth) {
    mTriangles = ExtractTriangles(mesh);
}

void BvhBuilder::Build(bool printStats) {
    mBvh.clear();
    mBvh.reserve(pow(2, mMaxDepth));  

    BvhNode root;
    root.TriangleCount = static_cast<uint32_t>(mTriangles.size());

    for (const auto& tri : mTriangles) {
        Grow(root, tri);
    }

    mBvh.push_back(root);

    BuildLayer(mBvh[0], 1);

    if (printStats) {
        PrintStats();
    }
}

void BvhBuilder::BuildLayer(BvhNode& parent, uint32_t depth) {
    if (depth >= mMaxDepth) {
        return;
    }

    parent.ChildIndex = static_cast<uint32_t>(mBvh.size());
    BvhNode& leftChild = mBvh.emplace_back();
    BvhNode& rightChild = mBvh.emplace_back();
    leftChild.TriangleIndex = parent.TriangleIndex;
    rightChild.TriangleIndex = parent.TriangleIndex;

    SplitAxis splitAxis;
    float splitPos;
    GetLongestAxis(parent, splitAxis, splitPos);

    for (uint32_t i = parent.TriangleIndex; i < parent.TriangleIndex + parent.TriangleCount; i++) {
        bool isLeft = IsLeft(mTriangles[i], splitAxis, splitPos);
        BvhNode& child = isLeft ? leftChild : rightChild;
        Grow(child, mTriangles[i]);
        child.TriangleCount++;

        if (isLeft) {
            int swapIndex = child.TriangleIndex + child.TriangleCount - 1;
            std::swap(mTriangles[i], mTriangles[swapIndex]);
            rightChild.TriangleIndex++;
        }
    }

    BuildLayer(leftChild, depth + 1);
    BuildLayer(rightChild, depth + 1);
}

void BvhBuilder::PrintStats() {
    uint32_t leavesCount = 0;
    uint32_t avgTriangles = 0;
    uint32_t maxTriangles = 0;
    uint32_t nonEmpty = 0;
    
    for (const auto& node : mBvh) {
        if (node.ChildIndex == 0) {
            leavesCount++;
            if (node.TriangleCount > 0) {
                nonEmpty++;
                avgTriangles += node.TriangleCount;
            }

            if (node.TriangleCount > maxTriangles) {
                maxTriangles = node.TriangleCount;
            }
        }
    }

    avgTriangles /= nonEmpty;

    LOG_INFO("Built BVH with depth: {}, total nodes: {}, leaves: {}, avg triangles per leaf (not empty): {}, max triangles in a leaf: {}",
        mMaxDepth, mBvh.size(), leavesCount, avgTriangles, maxTriangles);
}

void BvhBuilder::ExportToCSV(const std::string& filepath) const {
    std::ofstream file(filepath);
    file << "MinX,MinY,MinZ,MaxX,MaxY,MaxZ,ChildIndex,TriangleCount\n";
    for (size_t i = 0; i < mBvh.size(); ++i) {
        const auto& node = mBvh[i];
        file << node.Min.x << "," << node.Min.y << "," << node.Min.z << ","
             << node.Max.x << "," << node.Max.y << "," << node.Max.z << ","
             << node.ChildIndex << ","
             << node.TriangleCount << "\n";
    }
    file.close();
}
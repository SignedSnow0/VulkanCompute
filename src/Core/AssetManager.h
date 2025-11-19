#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Vulkan/Buffer.hpp"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanManager.h"

struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec2 uv;
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

private:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    friend class MeshRenderer;
};

class MeshRenderer {
public:
    MeshRenderer(const std::shared_ptr<VulkanManager> &vulkanManager,
                 const std::shared_ptr<Mesh> &mesh);

    [[nodiscard]] inline const Buffer<Vertex> *GetVertexBuffer() const {
        return mVertexBuffer.get();
    }
    [[nodiscard]] inline const Buffer<uint32_t> *GetIndexBuffer() const {
        return mIndexBuffer.get();
    }

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    std::shared_ptr<Mesh> mMesh;
    std::unique_ptr<Buffer<Vertex>> mVertexBuffer;
    std::unique_ptr<Buffer<uint32_t>> mIndexBuffer;
};

class Scene {
public:
    Scene() = default;

    [[nodiscard]] inline const std::vector<std::shared_ptr<Mesh>> &
    GetMeshes() const {
        return mMeshes;
    }

    void AddMesh(const std::shared_ptr<Mesh> &mesh);

private:
    std::vector<std::shared_ptr<Mesh>> mMeshes;
};

class AssetManager {
public:
    static std::shared_ptr<Scene> LoadScene(const std::string &filepath);

private:
};

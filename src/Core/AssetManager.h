#pragma once

#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <vector>

#include "Core/Model.h"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/VulkanManager.h"

struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec2 uv;
};

class AssetManager {
public:
    static std::shared_ptr<Mesh> LoadMesh(const std::string& filepath, const glm::mat4& modelMatrix = glm::mat4(1.0f));

private:
    static void ProcessNode(const aiScene* scene, const aiNode* node, Mesh* outMesh, const glm::mat4& modelMatrix);
    static std::vector<Triangle> ProcessMesh(const aiScene* scene, const aiMesh* mesh, const glm::mat4& modelMatrix);
};

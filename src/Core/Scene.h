#pragma once

#include <vector>
#include <memory>

#include "Core/Model.h"
#include "Core/BvhBuilder.h"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/VulkanManager.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Shader.h"

struct ModelUBO {
    uint32_t TriangleOffset;
    uint32_t BvhOffset;
    uint32_t MaterialIndex;
};

class Scene {
public:
    Scene(const std::shared_ptr<VulkanManager>& vulkanManager, const std::shared_ptr<Shader>& shader);

    void AddModel(const Model model);
    void AddSphere(const Sphere sphere);
    void AddPlane(const Plane plane);
    void AddMaterial(const Material material);

    [[nodiscard]] inline std::vector<Sphere>& Spheres() { return mSpheres; }
    [[nodiscard]] inline std::vector<Plane>& Planes() { return mPlanes; }
    [[nodiscard]] inline std::vector<Model>& Models() { return mModels; }
    [[nodiscard]] inline std::vector<Material>& Materials() { return mMaterials; }

    void Draw(const std::shared_ptr<CommandBuffer>& commandBuffer) const;

private:
    std::vector<Sphere> mSpheres;
    std::vector<Plane> mPlanes;
    std::vector<Model> mModels;
    std::vector<BvhNode> mBvhNodes;
    std::vector<Material> mMaterials;
    std::vector<Triangle> mTriangles;
    std::vector<ModelUBO> mModelUBOs;

    std::unique_ptr<StorageBuffer<Sphere>> mSpheresBuffer;
    std::unique_ptr<StorageBuffer<Plane>> mPlanesBuffer;
    std::unique_ptr<StorageBuffer<Triangle>> mTrianglesBuffer;
    std::unique_ptr<StorageBuffer<Material>> mMaterialsBuffer;
    std::unique_ptr<StorageBuffer<ModelUBO>> mModelUBOsBuffer;
    std::unique_ptr<StorageBuffer<BvhNode>> mBvhNodesBuffer;

    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Shader> mShader;
};
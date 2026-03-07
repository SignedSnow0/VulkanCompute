#pragma once

#include <vector>
#include <memory>

#include "Core/Model.h"
#include "Core/BvhBuilder.h"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/VulkanManager.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Shader.h"
#include "Core/VulkanComputeApp.h"

struct ModelUBO {
    uint32_t TriangleOffset;
    uint32_t BvhOffset;
    uint32_t MaterialIndex;
};

class Scene {
public:
    Scene(const std::shared_ptr<VulkanManager>& vulkanManager, const std::shared_ptr<Shader>& shader, VulkanComputeApp* app);

    void AddModel(Model model);
    void AddSphere(Sphere sphere, const Material material);
    void AddPlane(Plane plane, const Material material);

    void VisitSphere(std::function<bool(Sphere&, Material&)> func);
    void VisitPlane(std::function<bool(Plane&, Material&)> func);
    void VisitModel(std::function<bool(Model&, Material&)> func);

    void Draw(const std::shared_ptr<CommandBuffer>& commandBuffer);

private:
    bool mModifiedSpheres{ false };
    bool mModifiedPlanes{ false };
    bool mModifiedModels{ false };

    std::vector<Sphere> mSpheres;
    std::vector<Plane> mPlanes;
    std::vector<Triangle> mTriangles;
    std::vector<Model> mModels;
    std::vector<BvhNode> mBvhNodes;
    std::vector<Material> mMaterials;
    std::vector<ModelUBO> mModelUBOs;

    std::unique_ptr<StorageBuffer<Sphere>> mSpheresBuffer;
    std::unique_ptr<StorageBuffer<Plane>> mPlanesBuffer;
    std::unique_ptr<StorageBuffer<Triangle>> mTrianglesBuffer;
    std::unique_ptr<StorageBuffer<Material>> mMaterialsBuffer;
    std::unique_ptr<StorageBuffer<ModelUBO>> mModelUBOsBuffer;
    std::unique_ptr<StorageBuffer<BvhNode>> mBvhNodesBuffer;

    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Shader> mShader;
    VulkanComputeApp* mApp;

    bool mRebuild{ false };
    uint32_t mNumTriangles{ 0 };

    std::unique_ptr<ComputePipeline> mVertexPipeline;
};
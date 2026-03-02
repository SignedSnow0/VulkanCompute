#include "Scene.h"

static constexpr uint32_t MAX_BVH_DEPTH = 16;

Scene::Scene(const std::shared_ptr<VulkanManager>& vulkanManager, const std::shared_ptr<Shader>& shader)
    : mVulkanManager(vulkanManager), mShader(shader) {

}

void Scene::AddModel(const Model model) {
    uint32_t triangleOffset = mTrianglesBuffer ? static_cast<uint32_t>(mTrianglesBuffer->Size() / sizeof(Triangle)) : 0;
    auto bvhBuilder = BvhBuilder(model.GetMesh(), MAX_BVH_DEPTH);
    bvhBuilder.Build();

    ModelUBO modelUBO;
    modelUBO.TriangleOffset = triangleOffset;
    modelUBO.BvhOffset = mBvhNodes.size();
    modelUBO.MaterialIndex = mMaterials.size();

    mBvhNodes.insert(mBvhNodes.end(), bvhBuilder.GetBvh().begin(), bvhBuilder.GetBvh().end());
    mMaterials.push_back(model.GetMaterial());
    mModelUBOs.push_back(modelUBO);
    mModels.push_back(std::move(model));

    for (const auto& model : mModels) {
        const auto& mesh = bvhBuilder.GetTriangles();
        mTriangles.insert(mTriangles.end(), mesh.begin(), mesh.end());
    }

    mTrianglesBuffer = std::make_unique<StorageBuffer<Triangle>>(
        mVulkanManager, mTriangles.data(), mTriangles.size());
    mBvhNodesBuffer = std::make_unique<StorageBuffer<BvhNode>>(
        mVulkanManager, mBvhNodes.data(), mBvhNodes.size());
    mModelUBOsBuffer = std::make_unique<StorageBuffer<ModelUBO>>(
        mVulkanManager, mModelUBOs.data(), mModelUBOs.size());
    mMaterialsBuffer = std::make_unique<StorageBuffer<Material>>(
        mVulkanManager, mMaterials.data(), mMaterials.size());
}

void Scene::AddSphere(const Sphere sphere) {
    mSpheres.push_back(std::move(sphere));

    mSpheresBuffer = std::make_unique<StorageBuffer<Sphere>>(
        mVulkanManager, mSpheres.data(), mSpheres.size());
}

void Scene::AddPlane(const Plane plane) {
    mPlanes.push_back(std::move(plane));

    mPlanesBuffer = std::make_unique<StorageBuffer<Plane>>(
        mVulkanManager, mPlanes.data(), mPlanes.size());
}

void Scene::AddMaterial(const Material material) {
    mMaterials.push_back(std::move(material));

    mMaterialsBuffer = std::make_unique<StorageBuffer<Material>>(
        mVulkanManager, mMaterials.data(), mMaterials.size());
}

void Scene::Draw(const std::shared_ptr<CommandBuffer>& commandBuffer) const {
    mShader->BindStorageBuffer(*mSpheresBuffer, "spheresBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mPlanesBuffer, "planesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mMaterialsBuffer, "materialsBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mBvhNodesBuffer, "bvhNodesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mTrianglesBuffer, "trianglesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mModelUBOsBuffer, "modelsBuffer", commandBuffer->CurrentBufferIndex());
}
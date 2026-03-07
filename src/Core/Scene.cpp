#include "Scene.h"

static constexpr uint32_t MAX_BVH_DEPTH = 16;

Scene::Scene(const std::shared_ptr<VulkanManager>& vulkanManager,
             const std::shared_ptr<Shader>& shader,
             VulkanComputeApp* app)
    : mVulkanManager(vulkanManager), mShader(shader), mApp(app) {

    std::shared_ptr<Shader> vertexShader = std::shared_ptr<Shader>(
        Shader::Create(mVulkanManager, "assets/shaders/vertex.comp",
            ShaderStage::Compute, 1));

    mVertexPipeline = std::make_unique<ComputePipeline>(mVulkanManager, vertexShader);
}

void Scene::AddModel(Model model) {
    auto bvhBuilder = BvhBuilder(model, MAX_BVH_DEPTH);
    bvhBuilder.Build();

    ModelUBO modelUBO;
    modelUBO.TriangleOffset = mNumTriangles;
    modelUBO.BvhOffset = mBvhNodes.size();
    modelUBO.MaterialIndex = mMaterials.size();

    mBvhNodes.insert(mBvhNodes.end(), bvhBuilder.GetBvh().begin(), bvhBuilder.GetBvh().end());
    mMaterials.push_back(model.GetMaterial());
    mModelUBOs.push_back(modelUBO);
    mModels.push_back(std::move(model));

    for (const auto& model : mModels) {
        const auto& mesh = bvhBuilder.GetTriangles();
        mTriangles.insert(mTriangles.end(), mesh.begin(), mesh.end());
        mNumTriangles += mesh.size();
    }

    mRebuild = true;
}

void Scene::AddSphere(Sphere sphere, const Material material) {
    sphere.materialIndex = mMaterials.size();

    mSpheres.push_back(std::move(sphere));
    mMaterials.push_back(std::move(material));

    mRebuild = true;
}

void Scene::AddPlane(Plane plane, const Material material) {
    plane.materialIndex = mMaterials.size();

    mPlanes.push_back(std::move(plane));
    mMaterials.push_back(std::move(material));

    mRebuild = true;
}

void Scene::Draw(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    if (mRebuild) {
        for (uint32_t i = 0; i < mModels.size(); i++) {
            if (!mModels[i].GetUpdate()) {
                continue;
            }

            auto bvhBuilder = BvhBuilder(mModels[i], MAX_BVH_DEPTH);
            bvhBuilder.Build();

            auto& modelUBO = mModelUBOs[i];
            const auto& mesh = bvhBuilder.GetTriangles();
            const auto& bvh = bvhBuilder.GetBvh();
            mTriangles.insert(mTriangles.begin() + modelUBO.TriangleOffset, mesh.begin(), mesh.end());
            mBvhNodes.insert(mBvhNodes.begin() + modelUBO.BvhOffset, bvh.begin(), bvh.end());
        }
    
        mSpheresBuffer = std::make_unique<StorageBuffer<Sphere>>(
            mVulkanManager, mSpheres.data(), mSpheres.size());
        mPlanesBuffer = std::make_unique<StorageBuffer<Plane>>(
            mVulkanManager, mPlanes.data(), mPlanes.size());
        mTrianglesBuffer = std::make_unique<StorageBuffer<Triangle>>(
            mVulkanManager, mTriangles.data(), mTriangles.size());
        mBvhNodesBuffer = std::make_unique<StorageBuffer<BvhNode>>(
            mVulkanManager, mBvhNodes.data(), mBvhNodes.size());
        mModelUBOsBuffer = std::make_unique<StorageBuffer<ModelUBO>>(
            mVulkanManager, mModelUBOs.data(), mModelUBOs.size());
        mMaterialsBuffer = std::make_unique<StorageBuffer<Material>>(
            mVulkanManager, mMaterials.data(), mMaterials.size());

        mRebuild = false;
    }

    mShader->BindStorageBuffer(*mSpheresBuffer, "spheresBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mPlanesBuffer, "planesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mMaterialsBuffer, "materialsBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mBvhNodesBuffer, "bvhNodesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mTrianglesBuffer, "trianglesBuffer", commandBuffer->CurrentBufferIndex());
    mShader->BindStorageBuffer(*mModelUBOsBuffer, "modelsBuffer", commandBuffer->CurrentBufferIndex());
}

void Scene::VisitSphere(std::function<bool(Sphere&, Material&)> func) {
    bool modified = false;
    for (size_t i = 0; i < mSpheres.size(); ++i) {
        auto& sphere = mSpheres[i];
        auto& material = mMaterials[sphere.materialIndex];
        modified |= func(sphere, material);
    }

    mRebuild |= modified;
}

void Scene::VisitPlane(std::function<bool(Plane&, Material&)> func) {
    bool modified = false;
    for (size_t i = 0; i < mPlanes.size(); ++i) {
        auto& plane = mPlanes[i];
        auto& material = mMaterials[plane.materialIndex];
        modified |= func(plane, material);
    }

    mRebuild |= modified;
}

void Scene::VisitModel(std::function<bool(Model&, Material&)> func) {
    bool modified = false;
    for (size_t i = 0; i < mModels.size(); ++i) {
        auto& model = mModels[i];
        auto& material = mMaterials[i];
        modified |= func(model, material);
    }

    mRebuild |= modified;
}
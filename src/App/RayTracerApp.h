#pragma once

#include <random>
#include "App/UBOs.h"
#include "Core/VulkanComputeApp.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Shader.h"
#include "Core/AssetManager.h"

class RayTracerApp : public VulkanComputeApp {
public:
    RayTracerApp();
    ~RayTracerApp() override;

    void OnStart() override;
    void OnUpdate(float dt) override;
    void OnRender(float dt,
                  std::shared_ptr<CommandBuffer> commandBuffer) override;
    void OnStop() override;

private:
    void BuildScene();
    void RenderGui(Camera& camera, SceneData& sceneData, float dt);
    void BindUniformBuffers(const std::shared_ptr<CommandBuffer>& commandBuffer);
    void BindStorageBuffers(const std::shared_ptr<CommandBuffer>& commandBuffer);

    std::uniform_int_distribution<uint32_t> mRandomDistribution;
    std::mt19937 mRandomGenerator;
    std::shared_ptr<ComputePipeline> mPipeline;
    std::shared_ptr<Shader> mShader;

    std::shared_ptr<UniformBuffer<SceneData>> mSceneData;
    std::shared_ptr<UniformBuffer<Camera>> mCamera;

    std::shared_ptr<Buffer<Sphere>> mSpheresBuffer;
    std::shared_ptr<Buffer<Plane>> mPlanesBuffer;
    std::shared_ptr<Buffer<Material>> mMaterialsBuffer;

    std::vector<Sphere> mSpheres;
    std::vector<Plane> mPlanes;
    std::vector<Material> mMaterials;
    std::vector<MeshRenderer> mMeshes;
    std::shared_ptr<Scene> mScene;
};
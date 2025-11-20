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
    std::uniform_int_distribution<uint32_t> mRandomDistribution;
    std::mt19937 mRandomGenerator;
    std::shared_ptr<ComputePipeline> mPipeline;
    std::shared_ptr<Shader> mShader;
    std::shared_ptr<UniformBuffer<RandomSeed>> mSeed;
    std::shared_ptr<UniformBuffer<SceneData>> mSceneData;
    std::shared_ptr<UniformBuffer<Camera>> mCamera;

    std::shared_ptr<Scene> mScene;
    std::vector<MeshRenderer> mMeshes;
};
#pragma once

#include <random>

#include "App/UBOs.h"
#include "Core/AssetManager.h"
#include "Core/VulkanComputeApp.h"
#include "Core/Scene.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Shader.h"

class RayTracerApp : public VulkanComputeApp {
public:
    RayTracerApp();
    ~RayTracerApp() override;

    void OnStart() override;
    void OnUpdate(float dt) override;
    void OnRender(float dt,
                  std::shared_ptr<CommandBuffer> commandBuffer) override;
    void OnRenderGui(float dt) override;
    void OnStop() override;

private:
    void RenderViewport();
    void RenderSettings();
    void BuildScene();

    std::uniform_int_distribution<uint32_t> mRandomDistribution;
    std::mt19937 mRandomGenerator;
    std::shared_ptr<ComputePipeline> mPipeline;
    std::shared_ptr<Shader> mShader;

   
    std::shared_ptr<UniformBuffer<Camera>> mCamera;
    SceneData mSceneData{ 0, 0, 8, 16 };
    std::shared_ptr<UniformBuffer<SceneData>> mSceneDataBuffer;

    std::shared_ptr<Scene> mScene;
};
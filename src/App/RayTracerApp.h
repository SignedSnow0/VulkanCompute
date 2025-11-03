#pragma once

#include "App/UBOs.h"
#include "Core/VulkanComputeApp.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Shader.h"

class RayTracerApp : public VulkanComputeApp {
public:
    RayTracerApp();
    ~RayTracerApp() override;

    void OnStart() override;
    void OnUpdate(float dt) override;
    void OnRender(float dt, std::shared_ptr<CommandBuffer> commandBuffer) override;
    void OnStop() override;
private:
    std::shared_ptr<ComputePipeline> mPipeline;
    std::shared_ptr<Shader> mShader;
    std::shared_ptr <UniformBuffer<Camera>> mCamera;

};
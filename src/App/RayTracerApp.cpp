#include "RayTracerApp.h"

RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1280, 720, "Vulkan Ray Tracer") {
    mShader = std::make_shared<Shader>(
        mVulkanManager, "assets/shaders/RayTracer.comp", ShaderStage::Compute,
        mSurface->ImageCount());
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    mCamera = std::make_shared<UniformBuffer<Camera>>(mVulkanManager);
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {}

void RayTracerApp::OnUpdate(float dt) {
    static Camera cameraUBO{};

    if (mWindow.IsKeyPressed(GLFW_KEY_W)) {
        cameraUBO.position[2] -= 1.0f * dt;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_S)) {
        cameraUBO.position[2] += 1.0f * dt;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_A)) {
        cameraUBO.position[0] -= 1.0f * dt;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_D)) {
        cameraUBO.position[0] += 1.0f * dt;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_R)) {
        mVulkanManager->WaitIdle();

        mShader = std::make_shared<Shader>(
            mVulkanManager, "assets/shaders/RayTracer.comp",
            ShaderStage::Compute, mSurface->ImageCount());
        mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    }

    mCamera->UpdateData(cameraUBO);
}

void RayTracerApp::OnRender(float dt,
                            std::shared_ptr<CommandBuffer> commandBuffer) {
    mShader->BindSurfaceAsImage(mSurface, "framebuffer",
                                commandBuffer->CurrentBufferIndex());
    mShader->BindUniformBuffer(*mCamera, "camera",
                               commandBuffer->CurrentBufferIndex());

    mPipeline->Dispatch(commandBuffer, (mSurface->Extent().width + 7) / 8,
                        (mSurface->Extent().height + 7) / 8, 1,
                        commandBuffer->CurrentBufferIndex());
}

void RayTracerApp::OnStop() {}
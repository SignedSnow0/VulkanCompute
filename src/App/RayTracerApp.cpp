#include "RayTracerApp.h"

RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1280, 720, "Vulkan Ray Tracer") {
    mShader = std::shared_ptr<Shader>(
        Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                       ShaderStage::Compute, mSurface->ImageCount()));
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    mCamera = std::make_shared<UniformBuffer<Camera>>(mVulkanManager);
    mSettings = std::make_shared<UniformBuffer<Settings>>(mVulkanManager);
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {}

void RayTracerApp::OnUpdate(float dt) {
    static Camera cameraUBO{};
    static Settings settings{DebugView::None, 0.0f};

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

        auto *shader =
            Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                           ShaderStage::Compute, mSurface->ImageCount());
        if (shader) {
            mShader = std::shared_ptr<Shader>(shader);
        }

        mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    }

    settings.time += dt;
    if (mWindow.IsKeyPressed(GLFW_KEY_1)) {
        settings.debugView = DebugView::None;
    } else if (mWindow.IsKeyPressed(GLFW_KEY_2)) {
        settings.debugView = DebugView::Normals;
    } else if (mWindow.IsKeyPressed(GLFW_KEY_3)) {
        settings.debugView = DebugView::Depth;
    }

    mCamera->UpdateData(cameraUBO);
    mSettings->UpdateData(settings);
}

void RayTracerApp::OnRender(float dt,
                            std::shared_ptr<CommandBuffer> commandBuffer) {
    mShader->BindSurfaceAsImage(mSurface, "framebuffer",
                                commandBuffer->CurrentBufferIndex());
    mShader->BindUniformBuffer(*mCamera, "camera",
                               commandBuffer->CurrentBufferIndex());
    mShader->BindUniformBuffer(*mSettings, "settings",
                               commandBuffer->CurrentBufferIndex());

    if (mWindow.IsKeyPressed(GLFW_KEY_SPACE)) {
        return;
    }
    mPipeline->Dispatch(commandBuffer, (mSurface->Extent().width + 7) / 8,
                        (mSurface->Extent().height + 7) / 8, 1);
}

void RayTracerApp::OnStop() {}
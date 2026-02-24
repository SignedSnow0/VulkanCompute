#include "VulkanComputeApp.h"

#include <chrono>

VulkanComputeApp::VulkanComputeApp(uint32_t windowWidth, uint32_t windowHeight,
                                   const char *windowTitle)
    : mWindow{windowWidth, windowHeight, windowTitle} {
    mVulkanManager = std::make_shared<VulkanManager>(mWindow);
    mSurface = std::make_shared<Surface>(mVulkanManager, mWindow,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    mCommandBuffer =
        std::make_shared<CommandBuffer>(mVulkanManager, mSurface->ImageCount());
    mGui = std::make_shared<Gui>(mVulkanManager, mSurface, mWindow);

    mRendererImage = std::make_shared<Image>(mVulkanManager, VkExtent2D{ 1920, 1080 },
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_FORMAT_R8G8B8A8_UNORM);
    mRendererImageId = mGui->RegisterImage(mRendererImage);
}

VulkanComputeApp::~VulkanComputeApp() {}

void VulkanComputeApp::MainLoop() {
    OnStart();

    static auto lastTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(
                       currentTime - lastTime)
                       .count();
        lastTime = currentTime;
        
        for (const auto& task : mEndOfFrameTasks) {
            task(mCommandBuffer);
        }
        mEndOfFrameTasks.clear();

        mWindow.PollEvents();
        OnUpdate(dt);

        uint32_t imageIndex = mSurface->WaitNextImage();
        mCommandBuffer->Begin(imageIndex);

        OnRender(dt, mCommandBuffer);

        mGui->Begin(mCommandBuffer);
        OnRenderGui(dt);
        mGui->End(mCommandBuffer);

        mCommandBuffer->End();
        mSurface->SubmitCommandBuffer(mCommandBuffer, imageIndex);
    }

    mVulkanManager->WaitIdle();

    OnStop();
}

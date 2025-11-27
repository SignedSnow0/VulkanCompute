#include "VulkanComputeApp.h"

#include <chrono>

VulkanComputeApp::VulkanComputeApp(uint32_t windowWidth, uint32_t windowHeight,
                                   const char *windowTitle, uint32_t imageCount)
    : mWindow{windowWidth, windowHeight, windowTitle} {
    mVulkanManager = std::make_shared<VulkanManager>(mWindow);
    mSurface = std::make_shared<Surface>(mVulkanManager, mWindow,
                                         VK_IMAGE_LAYOUT_GENERAL, imageCount);
    mCommandBuffer =
        std::make_shared<CommandBuffer>(mVulkanManager, mSurface->ImageCount());

    mGui = std::make_shared<Gui>(mVulkanManager, mWindow, mSurface);
}

VulkanComputeApp::~VulkanComputeApp() {}

void VulkanComputeApp::MainLoop() {
    OnStart();

    static auto lastTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(
                       currentTime - lastTime)
                       .count();
        lastTime = currentTime;

        mWindow.pollEvents();
        mGui->Begin(mCommandBuffer);
        OnUpdate(dt);

        uint32_t imageIndex = mSurface->WaitNextImage();
        mCommandBuffer->Begin(imageIndex);
        mSurface->ChangeLayout(mCommandBuffer, VK_IMAGE_LAYOUT_GENERAL);

        OnRender(dt, mCommandBuffer);

        mSurface->ChangeLayout(mCommandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        mGui->End(mCommandBuffer);
        if (mShowGui) {
            mGui->Submit(mCommandBuffer);
        }

        mCommandBuffer->End();
        mSurface->SubmitCommandBuffer(mCommandBuffer, imageIndex);
    }

    mVulkanManager->WaitIdle();

    OnStop();
}

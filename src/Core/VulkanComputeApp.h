#pragma once

#include <memory>

#include "Core/Window.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Surface.h"
#include "Vulkan/VulkanManager.h"

class VulkanComputeApp {
public:
    VulkanComputeApp(uint32_t windowWidth, uint32_t windowHeight, const char* windowTitle);
    virtual ~VulkanComputeApp();

    virtual void OnStart() = 0;
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender(float dt, std::shared_ptr<CommandBuffer> commandBuffer) = 0;
    virtual void OnStop() = 0;

protected:
    Window mWindow;
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Surface> mSurface;

private:
    void Start();
    void MainLoop();
    void Stop();

    std::shared_ptr<RenderPass> mRenderPass;
    std::shared_ptr<ComputePipeline> mComputePipeline;
    std::shared_ptr<CommandBuffer> mCommandBuffer;

    friend int main(int argc, char** argv);
};
#pragma once
#include <memory>
#include "Core/Window.h"
#include "VulkanManager.h"
#include "Surface.h"
#include "RenderPass.h"
#include "CommandBuffer.h"

class Gui {
public:
    Gui(const std::shared_ptr<VulkanManager>& manager, Window& window, const std::shared_ptr<Surface>& surface);
    ~Gui();

    void Begin(const std::shared_ptr<CommandBuffer>& commandBuffer);
    void End(const std::shared_ptr<CommandBuffer>& commandBuffer);
    void Submit(const std::shared_ptr<CommandBuffer>& commandBuffer);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Surface> mSurface;
    std::shared_ptr<RenderPass> mRenderPass;
    std::shared_ptr<VkPipeline> mPipeline;

    VkDescriptorPool mDescriptorPool;
};
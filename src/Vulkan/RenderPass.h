#pragma once

#include <vulkan/vulkan.h>

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Surface.h"
#include "Vulkan/VulkanManager.h"

class RenderPass {
public:
    RenderPass(const std::shared_ptr<VulkanManager> &vulkanManager,
               const std::shared_ptr<Surface> &surface);
    ~RenderPass();

    [[nodiscard]] inline VkRenderPass RenderPassHandle() const {
        return mRenderPass;
    }
    [[nodiscard]] inline VkExtent2D Extent() const {
        return mSurface->Extent();
    }

    void Begin(const std::shared_ptr<CommandBuffer> &commandBuffer,
               uint32_t index);
    void End(const std::shared_ptr<CommandBuffer> &commandBuffer);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Surface> mSurface;

    VkRenderPass mRenderPass;
    std::vector<VkFramebuffer> mFramebuffers;
};

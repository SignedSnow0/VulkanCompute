#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "VulkanManager.h"
#include "Window.h"

class Surface {
  public:
    Surface(const std::shared_ptr<VulkanManager> &vulkanManager,
            const Window &window);
    ~Surface();

    [[nodiscard]] inline VkFormat Format() const { return mFormat; }
    [[nodiscard]] inline std::vector<VkImageView> ImageViews() const {
        return mSwapchainImageViews;
    }
    [[nodiscard]] inline VkExtent2D Extent() const { return mExtent; }
    [[nodiscard]] inline uint32_t ImageCount() const {
        return static_cast<uint32_t>(mSwapchainImages.size());
    }

    uint32_t WaitNextImage();
    void
    SubmitCommandBuffer(const std::shared_ptr<CommandBuffer> &commandBuffer,
                        uint32_t commandBufferIndex);
    void ChangeLayout(const std::shared_ptr<CommandBuffer> &commandBuffer,
                      VkImageLayout newLayout);

  private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkExtent2D mExtent;
    VkSurfaceKHR mSurface;
    VkFormat mFormat;
    VkSwapchainKHR mSwapchain;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    VkSemaphore mSemaphore;
    VkFence mFence;
    VkImageLayout mLayout;
};

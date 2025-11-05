#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "Core/Window.h"
#include "VulkanManager.h"

/**
 * @brief Class representing a Vulkan surface and swapchain.
 */
class Surface {
public:
    /**
     * @brief Constructs a Vulkan surface and swapchain for the given window.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param window Reference to the Window instance.
     * @param initialLayout Initial image layout for the swapchain images.
     */
    Surface(const std::shared_ptr<VulkanManager> &vulkanManager,
            const Window &window, VkImageLayout initialLayout);
    ~Surface();

    [[nodiscard]] inline VkFormat Format() const { return mFormat; }
    [[nodiscard]] inline std::vector<VkImage> Images() const {
        return mSwapchainImages;
    }
    [[nodiscard]] inline VkExtent2D Extent() const { return mExtent; }
    [[nodiscard]] inline uint32_t ImageCount() const {
        return static_cast<uint32_t>(mSwapchainImages.size());
    }
    [[nodiscard]] inline std::vector<VkImageLayout> ImageLayouts() const {
        return mLayouts;
    }
    [[nodiscard]] inline std::vector<VkImageView> ImageViews() const {
        return mSwapchainImageViews;
    }
    [[nodiscard]] inline std::vector<VkSampler> Samplers() const {
        return mSamplers;
    }

    /**
     * @brief Acquires the next available swapchain image in a synchronous
     * operation.
     *
     * @return Index of the retrieved image in the swapchain.
     */
    uint32_t WaitNextImage();
    /**
     * @brief Submits the given command buffer for execution and presents the
     * corresponding swapchain image.
     *
     * Before submitting the command buffer, its recording should be ended.
     *
     * @param commandBuffer Shared pointer to the CommandBuffer to submit.
     * @param commandBufferIndex Index of the command buffer to submit.
     */
    void
    SubmitCommandBuffer(const std::shared_ptr<CommandBuffer> &commandBuffer,
                        uint32_t commandBufferIndex);
    /**
     * @brief Changes the layout of the current swapchain image.
     *
     * @param commandBuffer Shared pointer to the CommandBuffer to use for the
     * layout transition.
     * @param newLayout The new image layout to transition to.
     */
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
    std::vector<VkSampler> mSamplers;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;
    std::vector<VkImageLayout> mLayouts;
    uint32_t mCurrentFrame = 0;
};

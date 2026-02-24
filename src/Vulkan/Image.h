#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

/**
 * @brief Class representing a Vulkan image with associated view and sampler.
 */
class Image {
public:
    Image(const std::shared_ptr<VulkanManager> &vulkanManager,
          VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL);
    ~Image();

    [[nodiscard]] inline VkImageView ImageView() const { return mImageView; }
    [[nodiscard]] inline VkSampler Sampler() const { return mSampler; }
    [[nodiscard]] inline VkImageLayout Layout() const { return mLayout; }
    [[nodiscard]] inline VkExtent2D Extent() const { return mExtent; }

    /**
     * @brief Changes the layout of the image.
     *
     * @param newLayout The new image layout to transition to.
     */
    void ChangeLayout(VkImageLayout newLayout);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkImage mImage;
    VkExtent2D mExtent;
    VkImageView mImageView;
    VkSampler mSampler;
    VkImageLayout mLayout;
    VkImageUsageFlags mUsage;
    VkFormat mFormat;
    VkDeviceMemory mImageMemory;

};

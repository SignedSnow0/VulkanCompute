#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

class Image {
public:
    Image(const std::shared_ptr<VulkanManager> &vulkanManager,
        VkExtent2D extent);
    Image(const std::shared_ptr<VulkanManager>& vulkanManager, const std::string& file);
    ~Image();

    [[nodiscard]] inline VkImageView ImageView() const { return mImageView; }
    [[nodiscard]] inline VkSampler Sampler() const { return mSampler; }
    [[nodiscard]] inline VkImageLayout Layout() const { return mLayout; }
    [[nodiscard]] inline VkExtent2D Extent() const { return mExtent; }

    void ChangeLayout(VkImageLayout newLayout);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkImage mImage;
    VkExtent2D mExtent;
    VkImageView mImageView;
    VkSampler mSampler;
    VkImageLayout mLayout;
};

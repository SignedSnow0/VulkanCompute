#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

class Image {
public:
    Image(const std::shared_ptr<VulkanManager> &vulkanManager,
          VkExtent2D extent);
    ~Image();

    [[nodiscard]] inline VkImageView ImageView() const { return mImageView; }
    [[nodiscard]] inline VkSampler Sampler() const { return mSampler; }

    void ChangeLayout(VkImageLayout newLayout);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkImage mImage;
    VkExtent2D mExtent;
    VkImageView mImageView;
    VkSampler mSampler;
    VkImageLayout mLayout;
};

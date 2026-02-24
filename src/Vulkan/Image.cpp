#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Vulkan/Utils.h"

VkImage createImage(VkDevice device, VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkImageLayout initialLayout) {
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent = {extent.width, extent.height, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usage;

    VkImage image;
    VK_CHECK(vkCreateImage(device, &createInfo, nullptr, &image));
    return image;
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    return imageView;
}

VkSampler createImageSampler(VkDevice device) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler imageSampler;
    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &imageSampler));
    return imageSampler;
}

VkDeviceMemory allocateImageMemory(const std::shared_ptr<VulkanManager> &vulkanManager, VkImage image) {
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkanManager->Device(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkanManager->PhysicalDevice(), memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkDeviceMemory imageMemory;
    VK_CHECK(vkAllocateMemory(vulkanManager->Device(), &allocInfo, nullptr, &imageMemory));
    VK_CHECK(vkBindImageMemory(vulkanManager->Device(), image, imageMemory, 0));

    return imageMemory;
}

Image::Image(const std::shared_ptr<VulkanManager> &vulkanManager,
             VkExtent2D extent, VkImageUsageFlags usage, VkFormat format, VkImageLayout initialLayout)
    : mVulkanManager(vulkanManager), mExtent(extent), mUsage(usage), mFormat(format) {
    mImage = createImage(vulkanManager->Device(), mExtent, mUsage, mFormat, mLayout);

    mImageMemory = allocateImageMemory(vulkanManager, mImage);
    mImageView = createImageView(vulkanManager->Device(), mImage, mFormat);
    mSampler = createImageSampler(vulkanManager->Device());

    mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ChangeLayout(initialLayout);
}

Image::~Image() {
    vkFreeMemory(mVulkanManager->Device(), mImageMemory, nullptr);
    vkDestroyImage(mVulkanManager->Device(), mImage, nullptr);
    vkDestroyImageView(mVulkanManager->Device(), mImageView, nullptr);
    vkDestroySampler(mVulkanManager->Device(), mSampler, nullptr);
}

void Image::ChangeLayout(VkImageLayout newLayout) {
    mVulkanManager->SubmitCommand([&](VkCommandBuffer commandBuffer) {
        changeLayout(commandBuffer, mLayout, newLayout, mImage);
        mLayout = newLayout;
    });
}
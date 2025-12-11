#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Vulkan/Utils.h"

VkImage createImage(VkDevice device, VkExtent2D extent, VkImageUsageFlags usage) {
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
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

VkImageView createImageView(VkDevice device, VkImage image) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    return imageView;
}

VkImage createImageToDeviceMemory(const std::shared_ptr<VulkanManager> &vulkanManager, VkBuffer buffer, VkImageLayout layout, VkExtent2D extent) {
    VkImage image = createImage(vulkanManager->Device(), extent, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vulkanManager->SubmitCommand([&](VkCommandBuffer commandBuffer) {
        if (!changeLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image)) {
            return;
        }

        copyBufferToImage(commandBuffer, buffer, image, extent, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout, image);
    }, CommandType::Transfer);

    vulkanManager->WaitIdle();

    return image;
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

Image::Image(const std::shared_ptr<VulkanManager> &vulkanManager,
             VkExtent2D extent)
    : mVulkanManager(vulkanManager), mExtent(extent) {
    mLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    mImage = createImage(vulkanManager->Device(), mExtent, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    mImageView = createImageView(vulkanManager->Device(), mImage);
    mSampler = createImageSampler(vulkanManager->Device());
}

Image::Image(const std::shared_ptr<VulkanManager> &vulkanManager, const std::string& file)
    : mVulkanManager(vulkanManager) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    mExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    mLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize imageSize = mExtent.width * mExtent.height * channels;

    createBuffer(vulkanManager, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanManager->Device(), stagingBufferMemory, 0, imageSize
        , 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(vulkanManager->Device(), stagingBufferMemory);
    
    stbi_image_free(pixels);

    mImage = createImageToDeviceMemory(vulkanManager, stagingBuffer, mLayout, mExtent);

    vulkanManager->WaitIdle();

    mImageView = createImageView(vulkanManager->Device(), mImage);
    mSampler = createImageSampler(vulkanManager->Device());
}

Image::~Image() {
    vkDestroyImage(mVulkanManager->Device(), mImage, nullptr);
    vkDestroyImageView(mVulkanManager->Device(), mImageView, nullptr);
    vkDestroySampler(mVulkanManager->Device(), mSampler, nullptr);
}

void Image::ChangeLayout(VkImageLayout newLayout) {
    mVulkanManager->SubmitCommand([&](VkCommandBuffer commandBuffer) {
        changeLayout(commandBuffer, mLayout, newLayout, mImage);
        mLayout = newLayout;
    }, CommandType::Graphics);
}

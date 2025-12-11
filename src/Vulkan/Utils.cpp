#include "Utils.h"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    return UINT32_MAX;
}

void createBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(
        vkCreateBuffer(vulkanManager->Device(), &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanManager->Device(), buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkanManager->PhysicalDevice(),
                       memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(vulkanManager->Device(), &allocInfo, nullptr,
                              &bufferMemory));
    VK_CHECK(
        vkBindBufferMemory(vulkanManager->Device(), buffer, bufferMemory, 0));
}

void copyBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    vulkanManager->SubmitCommand([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }, CommandType::Transfer);
}

void copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent2D extent, VkImageLayout layout) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {extent.width, extent.height, 1};

    vkCmdCopyBufferToImage(cmdBuffer, buffer, image,
                           layout, 1, &region);
}

bool changeLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout,
    VkImageLayout newLayout, VkImage image) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == newLayout) {
        return true;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.srcAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else {
        LOG_WARNING("Unsupported layout transition from {} to {}",
            string_VkImageLayout(oldLayout),
            string_VkImageLayout(newLayout));
        return false;
    }

    vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0,
        nullptr, 0, nullptr, 1, &barrier);

    return true;
}
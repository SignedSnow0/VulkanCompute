#include "Buffer.h"

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

void createBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer& buffer,
    VkDeviceMemory& bufferMemory) {
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

void copyBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    vulkanManager->SubmitCommand([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        });
}

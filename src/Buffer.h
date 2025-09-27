#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

void createBuffer(const std::shared_ptr<VulkanManager>& vulkanManager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void copyBuffer(const std::shared_ptr<VulkanManager>& vulkanManager, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
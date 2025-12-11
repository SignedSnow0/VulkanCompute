#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "Vulkan/VulkanManager.h"

void createBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory);

void copyBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void copyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkImage image, VkExtent2D extent, VkImageLayout layout);

bool changeLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout,
                  VkImageLayout newLayout, VkImage image);
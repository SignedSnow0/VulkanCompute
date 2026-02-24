#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "Vulkan/VulkanManager.h"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties);

void createBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory);

void copyBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void changeLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout,
                  VkImageLayout newLayout, VkImage image);
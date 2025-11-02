#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

void createBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer& buffer,
    VkDeviceMemory& bufferMemory);
void copyBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

template <typename T>
class UniformBuffer {
public:
    UniformBuffer(const std::shared_ptr<VulkanManager>& vulkanManager)
        : mVulkanManager(vulkanManager), mSize(sizeof(T)) {
        createBuffer(mVulkanManager, mSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mBuffer, mMemory);
    }

    ~UniformBuffer() {
        vkDestroyBuffer(mVulkanManager->Device(), mBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), mMemory, nullptr);
    }

    void UpdateData(const T& data) {
        void* mappedData;
        vkMapMemory(mVulkanManager->Device(), mMemory, 0, mSize, 0, &mappedData);
        memcpy(mappedData, &data, sizeof(T));
        vkUnmapMemory(mVulkanManager->Device(), mMemory);
    }

    VkBuffer Buffer() const { return mBuffer; }
    VkDeviceSize Size() const { return mSize; }

private:
    VkBuffer mBuffer;
    VkDeviceMemory mMemory;
    VkDeviceSize mSize;
    std::shared_ptr<VulkanManager> mVulkanManager;
};
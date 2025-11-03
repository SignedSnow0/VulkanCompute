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
class Buffer {
public:
    Buffer(const std::shared_ptr<VulkanManager>& vulkanManager,
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties)
        : mVulkanManager(vulkanManager), mSize(size) {
        createBuffer(mVulkanManager, size, usage, properties, mBuffer, mMemory);
    }

    Buffer(const std::shared_ptr<VulkanManager>& vulkanManager, const T* data, VkDeviceSize size, VkBufferUsageFlags usage)
        : mVulkanManager(vulkanManager), mSize(size) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(vulkanManager, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* mappedData;
        vkMapMemory(mVulkanManager->Device(), stagingBufferMemory, 0, size,
            0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(size));
        vkUnmapMemory(mVulkanManager->Device(), stagingBufferMemory);

        createBuffer(mVulkanManager, size,
            usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mBuffer, mMemory);

        copyBuffer(mVulkanManager, stagingBuffer, mBuffer, size);

        mVulkanManager->WaitIdle();

        vkDestroyBuffer(mVulkanManager->Device(), stagingBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), stagingBufferMemory, nullptr);
    }

    ~Buffer() {
        vkDestroyBuffer(mVulkanManager->Device(), mBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), mMemory, nullptr);
    }

    void UpdateData(const T* data, VkDeviceSize size, VkDeviceSize offset = 0) {
        void* mappedData;
        vkMapMemory(mVulkanManager->Device(), mMemory, offset, size, 0, &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(size));
        vkUnmapMemory(mVulkanManager->Device(), mMemory);
    }

    VkBuffer GetBuffer() const { return mBuffer; }
    VkDeviceSize Size() const { return mSize; }
private:
    VkBuffer mBuffer;
    VkDeviceMemory mMemory;
    VkDeviceSize mSize;
    std::shared_ptr<VulkanManager> mVulkanManager;
};

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
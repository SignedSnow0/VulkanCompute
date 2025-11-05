#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "Vulkan/Utils.h"
#include "Vulkan/VulkanManager.h"

/**
 * @brief Template class for a generic Vulkan buffer.
 */
template <typename T> class Buffer {
public:
    /**
     * @brief Constructs an empty Vulkan buffer of a specified size and usage.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param size Size of the buffer in bytes.
     * @param usage Usage flags for the buffer.
     * @param properties Memory property flags for the buffer.
     */
    Buffer(const std::shared_ptr<VulkanManager> &vulkanManager,
           VkDeviceSize size, VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties)
        : mVulkanManager(vulkanManager), mSize(size) {
        createBuffer(mVulkanManager, size, usage, properties, mBuffer, mMemory);
    }

    /**
     * @brief Constructs a Vulkan buffer and initializes it with provided data.
     *
     * This constructor assumes the data should not be updated after
     * construction.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param data Pointer to the data to initialize the buffer with.
     * @param size Size of the buffer in bytes.
     * @param usage Usage flags for the buffer.
     */
    Buffer(const std::shared_ptr<VulkanManager> &vulkanManager, const T *data,
           VkDeviceSize size, VkBufferUsageFlags usage)
        : mVulkanManager(vulkanManager), mSize(size) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(vulkanManager, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        void *mappedData;
        vkMapMemory(mVulkanManager->Device(), stagingBufferMemory, 0, size, 0,
                    &mappedData);
        memcpy(mappedData, data, static_cast<size_t>(size));
        vkUnmapMemory(mVulkanManager->Device(), stagingBufferMemory);

        createBuffer(mVulkanManager, size,
                     usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mBuffer, mMemory);

        copyBuffer(mVulkanManager, stagingBuffer, mBuffer, size);

        mVulkanManager->WaitIdle();

        vkDestroyBuffer(mVulkanManager->Device(), stagingBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), stagingBufferMemory, nullptr);
    }

    ~Buffer() {
        vkDestroyBuffer(mVulkanManager->Device(), mBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), mMemory, nullptr);
    }

    /**
     * @brief Updates the buffer data at a specified offset.
     *
     * Note: The buffer must be created with the generic constructor that does
     * not initialize the data.
     *
     * @param data Pointer to the data to copy into the buffer.
     * @param size Size of the data to copy in bytes.
     * @param offset Offset in the buffer where the data should be copied.
     */
    void UpdateData(const T *data, VkDeviceSize size, VkDeviceSize offset = 0) {
        void *mappedData;
        vkMapMemory(mVulkanManager->Device(), mMemory, offset, size, 0,
                    &mappedData);
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

/**
 * @brief Template class for a Vulkan uniform buffer.
 *
 * This class manages a Vulkan buffer specifically intended for use as a
 * uniform buffer (specified in glsl using the keyword "uniform").
 */
template <typename T> class UniformBuffer {
public:
    /**
     * @brief Constructs a Vulkan uniform buffer.
     *
     * The buffer is intended to hold only a single instance of the type T.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     */
    UniformBuffer(const std::shared_ptr<VulkanManager> &vulkanManager)
        : mVulkanManager(vulkanManager), mSize(sizeof(T)) {
        createBuffer(mVulkanManager, mSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     mBuffer, mMemory);
    }

    ~UniformBuffer() {
        vkDestroyBuffer(mVulkanManager->Device(), mBuffer, nullptr);
        vkFreeMemory(mVulkanManager->Device(), mMemory, nullptr);
    }

    /**
     * @brief Updates the uniform buffer with new data to be sent to the GPU.
     *
     * @param data Reference to the data to copy into the buffer.
     */
    void UpdateData(const T &data) {
        void *mappedData;
        vkMapMemory(mVulkanManager->Device(), mMemory, 0, mSize, 0,
                    &mappedData);
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
#pragma once

#include <functional>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "Core/Logger.h"
#include "Core/Window.h"

#define VK_CHECK(fn)                                                           \
    {                                                                          \
        ::VkResult res = (fn);                                                 \
        if (res != VK_SUCCESS) {                                               \
            LOG_WARNING("Vulkan call error: VkResult is {}",                   \
                        string_VkResult(res));                                 \
        }                                                                      \
    }

struct QueueInfo {
    VkQueue compute;
    uint32_t computeFamilyIndex;
    VkCommandPool computeCommandPool;
    VkCommandBuffer computeCommandBuffer;

    VkQueue graphics;
    uint32_t graphicsFamilyIndex;
    VkCommandPool graphicsCommandPool;
    VkCommandBuffer graphicsCommandBuffer;

    VkQueue transfer;
    uint32_t transferFamilyIndex;
    VkCommandPool transferCommandPool;
    VkCommandBuffer transferCommandBuffer;
};

enum class CommandType {
    Compute,
    Graphics,
    Transfer
};

class VulkanManager {
public:
    VulkanManager(Window &window);
    ~VulkanManager();

    [[nodiscard]] inline VkInstance Instance() const { return mInstance; }
    [[nodiscard]] inline VkPhysicalDevice PhysicalDevice() const {
        return mPhysicalDevice;
    }
    [[nodiscard]] inline VkDevice Device() const { return mDevice; }
    [[nodiscard]] inline VkQueue ComputeQueue() const { return mQueues.compute; }
    [[nodiscard]] inline uint32_t ComputeQueueFamilyIndex() const { return mQueues.computeFamilyIndex; }
    [[nodiscard]] inline VkQueue GraphicsQueue() const { return mQueues.graphics; }
    [[nodiscard]] inline uint32_t GraphicsQueueFamilyIndex() const { return mQueues.graphicsFamilyIndex; }
    [[nodiscard]] inline VkQueue TransferQueue() const { return mQueues.transfer; }
    [[nodiscard]] inline uint32_t TransferQueueFamilyIndex() const { return mQueues.transferFamilyIndex; }

    void WaitIdle() const;
    void SubmitCommand(std::function<void(VkCommandBuffer)> func, CommandType type);

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;

    QueueInfo mQueues;
};

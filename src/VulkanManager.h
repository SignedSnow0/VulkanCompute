#pragma once

#include <vulkan/vulkan.h>

#include "Window.h"

class VulkanManager
{
public:
    VulkanManager(Window& window);
    ~VulkanManager();

    [[nodiscard]] inline VkInstance Instance() const { return mInstance; }
    [[nodiscard]] inline VkPhysicalDevice PhysicalDevice() const { return mPhysicalDevice; }
    [[nodiscard]] inline VkDevice Device() const { return mDevice; }
    [[nodiscard]] inline VkQueue ComputeQueue() const { return mComputeQueue; }
    [[nodiscard]] inline uint32_t ComputeQueueFamilyIndex() const { return 0; } // Placeholder, should be set during physical device selection

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
    VkQueue mComputeQueue;
};
#pragma once

#include <functional>
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
    [[nodiscard]] inline uint32_t ComputeQueueFamilyIndex() const { return 0; }

    void SubmitCommnand(std::function<void(VkCommandBuffer)> func);

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
    VkQueue mComputeQueue;

    VkCommandPool mCommandPool;
    VkCommandBuffer mCommandBuffer;
};
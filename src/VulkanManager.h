#pragma once

#include <functional>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "Logger.h"
#include "Window.h"

#define VK_CHECK(fn)                                                           \
    {                                                                          \
        ::VkResult res = (fn);                                                 \
        if (res != VK_SUCCESS) {                                               \
            LOG_WARNING("Vulkan call error: VkResult is {}",                   \
                                    string_VkResult(res));                     \
        }                                                                      \
    }

class VulkanManager {
public:
    VulkanManager(Window& window);
    ~VulkanManager();

    [[nodiscard]] inline VkInstance Instance() const { return mInstance; }
    [[nodiscard]] inline VkPhysicalDevice PhysicalDevice() const {
        return mPhysicalDevice;
    }
    [[nodiscard]] inline VkDevice Device() const { return mDevice; }
    [[nodiscard]] inline VkQueue ComputeQueue() const { return mComputeQueue; }
    [[nodiscard]] inline uint32_t ComputeQueueFamilyIndex() const { return 0; }

    void WaitIdle() const;
    void SubmitCommand(std::function<void(VkCommandBuffer)> func);

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
    VkQueue mComputeQueue;

    VkCommandPool mCommandPool;
    VkCommandBuffer mCommandBuffer;
};

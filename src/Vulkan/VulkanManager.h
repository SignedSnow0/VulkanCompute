#pragma once

#include <functional>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include "Core/Logger.h"
#include "Core/Window.h"

/**
 * @brief Macro to check the result of Vulkan function calls.
 *
 * If the result is not VK_SUCCESS, a warning is logged with the corresponding
 * VkResult string.
 */
#define VK_CHECK(fn)                                                           \
    {                                                                          \
        ::VkResult res = (fn);                                                 \
        if (res != VK_SUCCESS) {                                               \
            LOG_WARNING("Vulkan call error: VkResult is {}",                   \
                        string_VkResult(res));                                 \
        }                                                                      \
    }

/**
 * @brief Class managing Vulkan instance, physical device, logical device,
 * and queues.
 */
class VulkanManager {
public:
    /**
     * @brief Constructs a VulkanManager for the given window.
     *
     * @param window Reference to the Window instance.
     */
    VulkanManager(Window &window);
    ~VulkanManager();

    [[nodiscard]] inline VkInstance Instance() const { return mInstance; }
    [[nodiscard]] inline VkPhysicalDevice PhysicalDevice() const {
        return mPhysicalDevice;
    }
    [[nodiscard]] inline VkDevice Device() const { return mDevice; }
    [[nodiscard]] inline VkQueue ComputeQueue() const { return mComputeQueue; }
    [[nodiscard]] inline uint32_t ComputeQueueFamilyIndex() const { return 0; }

    /**
     * @brief Waits for the device to finish all operations.
     */
    void WaitIdle() const;
    /**
     * @brief Instantly submits commands to the device.
     *
     * @param func Function that takes a VkCommandBuffer and records commands
     * into it.
     */
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

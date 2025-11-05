#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

/**
 * @brief Class representing a Vulkan command buffer.
 *
 * This class manages a pool of Vulkan command buffers, allowing for recording
 * and executing commands.
 */
class CommandBuffer {
public:
    /**
     * @brief Constructs a Vulkan command buffer manager.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param count Number of command buffers to create.
     */
    CommandBuffer(const std::shared_ptr<VulkanManager> &vulkanManager,
                  uint32_t count);
    ~CommandBuffer();

    [[nodiscard]] VkCommandBuffer CurrentBuffer() const;
    [[nodiscard]] uint32_t CurrentBufferIndex() const;
    [[nodiscard]] inline std::vector<VkCommandBuffer> Buffers() const {
        return mCommandBuffers;
    }

    /**
     * @brief Begins recording commands into the command buffer at the given
     * index.
     *
     * This method should be called before recording any commands into the
     * buffer.
     *
     * @param index Index of the command buffer to begin recording. Usually
     * given by the Surface's image index.
     */
    void Begin(uint32_t index);
    /**
     * @brief Ends recording commands into the current command buffer.
     *
     * This method should be called after all commands have been recorded into
     * the buffer.
     */
    void End();
    /**
     * @brief Executes a function that records commands into the current command
     * buffer.
     *
     * If the command buffer is not currently being recorded, a warning is
     * issued.
     *
     * @param func Function that takes a VkCommandBuffer and records commands
     * into it.
     */
    void
    ExecuteCommand(std::function<void(VkCommandBuffer commandBuffer)> func);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkCommandPool mPool;
    std::vector<VkCommandBuffer> mCommandBuffers;
    std::optional<uint32_t> mCurrentBufferIndex;
};

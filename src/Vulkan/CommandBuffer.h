#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

class CommandBuffer {
public:
  CommandBuffer(const std::shared_ptr<VulkanManager>& vulkanManager,
    uint32_t count);
  ~CommandBuffer();

  [[nodiscard]] VkCommandBuffer CurrentBuffer() const;
  [[nodiscard]] uint32_t CurrentBufferIndex() const;
  [[nodiscard]] inline std::vector<VkCommandBuffer> Buffers() const {
    return mCommandBuffers;
  }

  void Begin(uint32_t index);
  void End();
  void ExecuteCommand(std::function<void(VkCommandBuffer commandBuffer)> func);

private:
  std::shared_ptr<VulkanManager> mVulkanManager;

  VkCommandPool mPool;
  std::vector<VkCommandBuffer> mCommandBuffers;
  std::optional<uint32_t> mCurrentBufferIndex;
};

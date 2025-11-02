#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "Image.h"
#include "Surface.h"
#include "VulkanManager.h"

enum ShaderStage { Vertex, Fragment, Compute };

class Shader {
public:
  Shader(const std::shared_ptr<VulkanManager>& vulkanManager,
    const std::string& filename, ShaderStage stage, uint32_t setCount);
  ~Shader();

  [[nodiscard]] inline VkDescriptorSetLayout DescriptorSetLayout(uint32_t frameIndex) const {
    return mDescriptorSetLayouts[frameIndex];
  }
  [[nodiscard]] inline VkDescriptorSet DescriptorSet(uint32_t frameIndex) const {
    return mDescriptorSets[frameIndex];
  }
  [[nodiscard]] VkPipelineShaderStageCreateInfo CreateShaderStageInfo() const;

  void BindImage(const Image& image, uint32_t binding, uint32_t frameIndex);
  void BindSurfaceAsImage(const std::shared_ptr<Surface>& surface,
    uint32_t binding, uint32_t index);

private:
  std::shared_ptr<VulkanManager> mVulkanManager;

  VkShaderStageFlagBits mStage;
  VkShaderModule mShader;
  std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
  VkDescriptorPool mDescriptorPool;
  std::vector<VkDescriptorSet> mDescriptorSets;
};

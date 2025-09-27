#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"
#include "CommandBuffer.h"
#include "Image.h"
#include "Surface.h"

enum ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

class Shader
{
public:
    Shader(const std::shared_ptr<VulkanManager>& vulkanManager, const std::string& filename, ShaderStage stage, uint32_t setCount);
    ~Shader();

    [[nodiscard]] inline VkDescriptorSetLayout DescriptorSetLayout() const { return mDescriptorSetLayout; }
    [[nodiscard]] inline VkDescriptorSet DescriptorSet() const { return mDescriptorSet; }
    [[nodiscard]] VkPipelineShaderStageCreateInfo CreateShaderStageInfo() const;

    void BindImage(const Image& image, uint32_t binding);
    void BindSurfaceAsImage(const std::shared_ptr<Surface>& surface, uint32_t binding, uint32_t index);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkShaderStageFlagBits mStage;
    VkShaderModule mShader;
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSet mDescriptorSet;
};
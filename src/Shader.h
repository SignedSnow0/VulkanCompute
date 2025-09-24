#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"

enum ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

class Shader
{
public:
    Shader(const std::shared_ptr<VulkanManager>& vulkanManager, const std::string& filename, ShaderStage stage);
    ~Shader();

    VkPipelineShaderStageCreateInfo CreateShaderStageInfo() const;

private:
    std::shared_ptr<VulkanManager> mVulkanManager;

    VkShaderStageFlagBits mStage;
    VkShaderModule mShader;
};
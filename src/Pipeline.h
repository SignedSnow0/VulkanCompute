#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanManager.h"
#include "Shader.h"
#include "RenderPass.h"
#include "CommandBuffer.h"

class Pipeline
{
public:
    Pipeline(const std::shared_ptr<VulkanManager>& vulkanManager, const std::vector<std::shared_ptr<Shader>>& shaders, const std::shared_ptr<RenderPass>& renderPass);
    ~Pipeline();

    void Bind(const std::shared_ptr<CommandBuffer>& commandBuffer);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<RenderPass> mRenderPass;
    std::vector<std::shared_ptr<Shader>> mShaders;

    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};
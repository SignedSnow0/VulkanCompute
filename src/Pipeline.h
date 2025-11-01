#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "RenderPass.h"
#include "Shader.h"
#include "VulkanManager.h"

class Pipeline {
  public:
    Pipeline(const std::shared_ptr<VulkanManager> &vulkanManager,
             const std::vector<std::shared_ptr<Shader>> &shaders,
             const std::shared_ptr<RenderPass> &renderPass);
    ~Pipeline();

    void Bind(const std::shared_ptr<CommandBuffer> &commandBuffer);

  private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<RenderPass> mRenderPass;
    std::vector<std::shared_ptr<Shader>> mShaders;

    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};

class ComputePipeline {
  public:
    ComputePipeline(const std::shared_ptr<VulkanManager> &vulkanManager,
                    const std::shared_ptr<Shader> &computeShader);
    ~ComputePipeline();

    void Dispatch(const std::shared_ptr<CommandBuffer> &commandBuffer,
                  uint32_t groupCountX, uint32_t groupCountY,
                  uint32_t groupCountZ);

  private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Shader> mComputeShader;

    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};

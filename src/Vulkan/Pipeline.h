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

/**
 * @brief Class representing a Vulkan compute pipeline.
 */
class ComputePipeline {
public:
    /**
     * @brief Constructs a Vulkan compute pipeline.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param computeShader Shared pointer to the compute Shader instance.
     */
    ComputePipeline(const std::shared_ptr<VulkanManager> &vulkanManager,
                    const std::shared_ptr<Shader> &computeShader);
    ~ComputePipeline();

    /**
     * @brief Dispatches compute work using the pipeline.
     *
     * In general, the number of workgroups dispatched should cover the total
     * number of invocations needed for the compute shader.
     *
     * @param commandBuffer Shared pointer to the CommandBuffer to record
     * commands into.
     * @param groupCountX Number of workgroups to dispatch in the X dimension.
     * @param groupCountY Number of workgroups to dispatch in the Y dimension.
     * @param groupCountZ Number of workgroups to dispatch in the Z dimension.
     */
    void Dispatch(const std::shared_ptr<CommandBuffer> &commandBuffer,
                  uint32_t groupCountX, uint32_t groupCountY,
                  uint32_t groupCountZ);

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Shader> mComputeShader;

    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};

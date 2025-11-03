#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "Image.h"
#include "Surface.h"
#include "VulkanManager.h"
#include "Buffer.h"

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

    template <typename T>
    void BindBuffer(const Buffer<T>& buffer, uint32_t binding, uint32_t frameIndex) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffer.GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = buffer.Size();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = mDescriptorSets[frameIndex];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
            nullptr);
    }

    template <typename T>
    void BindUniformBuffer(const UniformBuffer<T>& uniformBuffer,
        uint32_t binding, uint32_t frameIndex) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffer.Buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = uniformBuffer.Size();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = mDescriptorSets[frameIndex];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
            nullptr);
    }
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

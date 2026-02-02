#pragma once

#include <map>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "Vulkan/Buffer.hpp"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Surface.h"
#include "Vulkan/VulkanManager.h"

enum class ShaderStage { Vertex, Fragment, Compute };

class Shader {
public:
    static Shader *Create(const std::shared_ptr<VulkanManager> &vulkanManager,
                          const std::string &filename, ShaderStage stage,
                          uint32_t setCount);
    ~Shader();

    [[nodiscard]] inline VkDescriptorSetLayout
    DescriptorSetLayout(uint32_t frameIndex) const {
        return mDescriptorSetLayouts[frameIndex];
    }
    [[nodiscard]] inline VkDescriptorSet
    DescriptorSet(uint32_t frameIndex) const {
        return mDescriptorSets[frameIndex];
    }
    [[nodiscard]] VkPipelineShaderStageCreateInfo CreateShaderStageInfo() const;

    [[nodiscard]] inline uint32_t GroupSizeX() const { return mWorkGroupSizeX; }
    [[nodiscard]] inline uint32_t GroupSizeY() const { return mWorkGroupSizeY; }
    [[nodiscard]] inline uint32_t GroupSizeZ() const { return mWorkGroupSizeZ; }

    [[nodiscard]] inline glm::uvec3 GroupSize() const {
        return glm::uvec3{
            mWorkGroupSizeX,
            mWorkGroupSizeY,
            mWorkGroupSizeZ
        };
    }

    [[nodiscard]] inline glm::uvec3 FitInvocationToWorkGroup(const glm::uvec3& invocation) const {
        return glm::uvec3{
            (invocation.x + mWorkGroupSizeX - 1) / mWorkGroupSizeX,
            (invocation.y + mWorkGroupSizeY - 1) / mWorkGroupSizeY,
            (invocation.z + mWorkGroupSizeZ - 1) / mWorkGroupSizeZ
        };
    }

    template <typename T>
    void BindBuffer(const Buffer<T> &buffer, uint32_t binding,
                    uint32_t frameIndex) {
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
    void BindBuffer(const Buffer<T> &buffer, const std::string &name,
                    uint32_t frameIndex) {
        auto it = mBindingMap.find(name);
        if (it == mBindingMap.end()) {
            LOG_WARNING("Failed to bind buffer: no binding found for name '{}'",
                        name);
            return;
        }

        BindBuffer(buffer, it->second, frameIndex);
    }

    template <typename T>
    void BindUniformBuffer(const UniformBuffer<T> &uniformBuffer,
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
    template <typename T>
    void BindUniformBuffer(const UniformBuffer<T> &uniformBuffer,
                           const std::string &name, uint32_t frameIndex) {
        auto it = mBindingMap.find(name);
        if (it == mBindingMap.end()) {
            LOG_WARNING(
                "Failed to bind uniform buffer: no binding found for name '{}'",
                name);
            return;
        }

        BindUniformBuffer(uniformBuffer, it->second, frameIndex);
    }

    void BindImage(const Image &image, uint32_t binding, uint32_t frameIndex, bool isStorage = true);
    void BindImage(const Image &image, const std::string &name,
                   uint32_t frameIndex, bool isStorage = true) {
        auto it = mBindingMap.find(name);
        if (it == mBindingMap.end()) {
            LOG_WARNING("Failed to bind image: no binding found for name '{}'",
                        name);
            return;
        }

        BindImage(image, it->second, frameIndex, isStorage);
    }

    void BindSurfaceAsImage(const std::shared_ptr<Surface> &surface,
                            uint32_t binding, uint32_t index);
    void BindSurfaceAsImage(const std::shared_ptr<Surface> &surface,
                            const std::string &name, uint32_t index) {
        auto it = mBindingMap.find(name);
        if (it == mBindingMap.end()) {
            LOG_WARNING(
                "Failed to bind surface image: no binding found for name '{}'",
                name);
            return;
        }

        BindSurfaceAsImage(surface, it->second, index);
    }

private:
    Shader(const std::shared_ptr<VulkanManager> &vulkanManager,
           const std::string &filename, ShaderStage stage, uint32_t setCount);

    std::shared_ptr<VulkanManager> mVulkanManager;

    uint32_t mWorkGroupSizeX{0};
    uint32_t mWorkGroupSizeY{0};
    uint32_t mWorkGroupSizeZ{0};

    VkShaderStageFlagBits mStage;
    VkShaderModule mShader{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
    VkDescriptorPool mDescriptorPool;
    std::vector<VkDescriptorSet> mDescriptorSets;
    std::map<std::string, uint32_t> mBindingMap;
};

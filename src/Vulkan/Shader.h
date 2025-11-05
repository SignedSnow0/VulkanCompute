#pragma once

#include <map>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

#include "Vulkan/Buffer.hpp"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Surface.h"
#include "Vulkan/VulkanManager.h"

/**
 * @brief Enumeration of shader stages.
 */
enum ShaderStage { Vertex, Fragment, Compute };

/**
 * @brief Class representing a Vulkan shader.
 *
 * This class creates a Vulkan shader and its reflection from a GLSL source
 * file.
 */
class Shader {
public:
    /**
     * @brief Creates a Vulkan shader from a GLSL source file.
     *
     * This function compiles the GLSL source code into a SPIR-V module and
     * creates a Vulkan shader object. If the source code cannot be compiled,
     * the application will log the compilation error and return nullptr.
     *
     * @param vulkanManager Shared pointer to the VulkanManager instance.
     * @param filename Path to the GLSL source file.
     * @param stage Shader stage (Vertex, Fragment, Compute).
     * @param setCount Number of descriptor sets used by the shader.
     * @return Pointer to the created Shader instance (or nullptr on compilation
     * error).
     */
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

    /**
     * @brief Binds a buffer to the shader's descriptor set at the specified
     * binding point.
     *
     * @tparam T Type of the buffer data.
     * @param buffer Reference to the Buffer instance to bind.
     * @param binding Binding point in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the buffer
     * to.
     */
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
    /**
     * @brief Binds a buffer to the shader's descriptor set using the
     * name of the buffer in the shader code.
     *
     * @tparam T Type of the buffer data.
     * @param buffer Reference to the Buffer instance to bind.
     * @param name Binding name in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the buffer
     * to.
     */
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

    /**
     * @brief Binds a uniform buffer to the shader's descriptor set at the
     * specified binding point.
     *
     * @tparam T Type of the uniform buffer data.
     * @param uniformBuffer Reference to the UniformBuffer instance to bind.
     * @param binding Binding point in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the uniform
     * buffer to.
     */
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
    /**
     * @brief Binds a uniform buffer to the shader's descriptor set using the
     * name of the uniform in the shader code.
     *
     * @tparam T Type of the uniform buffer data.
     * @param uniformBuffer Reference to the UniformBuffer instance to bind.
     * @param name Binding name in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the uniform
     * buffer to.
     */
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

    /**
     * @brief Binds an image to the shader's descriptor set at the specified
     * binding point.
     *
     * @param image Reference to the Image instance to bind.
     * @param binding Binding point in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the image
     * to.
     */
    void BindImage(const Image &image, uint32_t binding, uint32_t frameIndex);
    /**
     * @brief Binds an image to the shader's descriptor set using the
     * name of the image in the shader code.
     *
     * @param image Reference to the Image instance to bind.
     * @param name Binding name in the shader.
     * @param frameIndex Index of the frame (descriptor set) to bind the image
     * to.
     */
    void BindImage(const Image &image, const std::string &name,
                   uint32_t frameIndex);

    /**
     * @brief Binds a surface image to the shader's descriptor set at the
     * specified binding point.
     *
     * @param surface Shared pointer to the Surface instance.
     * @param binding Binding point in the shader.
     * @param index Index of the frame (descriptor set) to bind the surface
     * image to.
     */
    void BindSurfaceAsImage(const std::shared_ptr<Surface> &surface,
                            uint32_t binding, uint32_t index);
    /**
     * @brief  Binds a surface image to the shader's descriptor set using the
     * name of the image in the shader code.
     *
     * @param surface Shared pointer to the Surface instance.
     * @param name Binding name in the shader.
     * @param index Index of the frame (descriptor set) to bind the surface
     * image to.
     */
    void BindSurfaceAsImage(const std::shared_ptr<Surface> &surface,
                            const std::string &name, uint32_t index);

private:
    Shader(const std::shared_ptr<VulkanManager> &vulkanManager,
           const std::string &filename, ShaderStage stage, uint32_t setCount);

    std::shared_ptr<VulkanManager> mVulkanManager;

    VkShaderStageFlagBits mStage;
    VkShaderModule mShader{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
    VkDescriptorPool mDescriptorPool;
    std::vector<VkDescriptorSet> mDescriptorSets;
    std::map<std::string, uint32_t> mBindingMap;
};

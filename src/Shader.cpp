#include "Shader.h"

#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <vector>

shaderc_shader_kind getShaderKind(ShaderStage stage) {
    switch (stage) {
    case Vertex:
        return shaderc_vertex_shader;
    case Fragment:
        return shaderc_fragment_shader;
    case Compute:
        return shaderc_compute_shader;
    default:
        return shaderc_glsl_infer_from_source;
    }
}

VkShaderStageFlagBits getVulkanShaderStage(ShaderStage stage) {
    switch (stage) {
    case Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        return VK_SHADER_STAGE_ALL; // Fallback
    }
}

std::vector<uint32_t> compileShader(const std::string& filename,
    shaderc_shader_kind kind) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARNING(std::format("Failed to open shader file: {}", filename));
        return {};
    }

    std::string source((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(source, kind, filename.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG_WARNING(std::format("Error compiling shader {}: {}", filename,
            module.GetErrorMessage()));
        return {};
    }

    return std::vector<uint32_t>(module.cbegin(), module.cend());
}

VkShaderModule createShaderModule(VkDevice device,
    const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

void createDescriptorSet(VkDevice device, const std::vector<uint32_t>& bytecode,
    VkShaderStageFlagBits stage, uint32_t setCount,
    VkDescriptorSetLayout& outLayout,
    VkDescriptorPool& outPool, VkDescriptorSet& outSet) {
    spirv_cross::Compiler glslCompiler(bytecode);
    spirv_cross::ShaderResources resources =
        glslCompiler.get_shader_resources();

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& resource : resources.storage_buffers) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);
    }

    for (const auto& resource : resources.sampled_images) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);
    }

    for (const auto& resource : resources.uniform_buffers) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);
    }

    for (const auto& resource : resources.storage_images) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
        &outLayout));

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto& binding : bindings) {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = binding.descriptorType;
        poolSize.descriptorCount = binding.descriptorCount * setCount;
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = setCount;

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &outPool));

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = outPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &outLayout;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &outSet));
}

Shader::Shader(const std::shared_ptr<VulkanManager>& vulkanManager,
    const std::string& filename, ShaderStage stage,
    uint32_t setCount)
    : mVulkanManager(vulkanManager), mStage(getVulkanShaderStage(stage)) {
    auto spirv = compileShader(filename, getShaderKind(stage));
    mShader = createShaderModule(mVulkanManager->Device(), spirv);

    createDescriptorSet(mVulkanManager->Device(), spirv, mStage, setCount,
        mDescriptorSetLayout, mDescriptorPool, mDescriptorSet);

    LOG_INFO(std::format("Created shader: {}", filename));
}

Shader::~Shader() {
    vkDestroyShaderModule(mVulkanManager->Device(), mShader, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::CreateShaderStageInfo() const {
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = mStage;
    shaderStageInfo.module = mShader;
    shaderStageInfo.pName = "main";                // Entry point
    shaderStageInfo.pSpecializationInfo = nullptr; // Optional

    return shaderStageInfo;
}

void Shader::BindImage(const Image& image, uint32_t binding) {
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = image.ImageView();
    imageInfo.sampler = image.Sampler();

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
        nullptr);
}

void Shader::BindSurfaceAsImage(const std::shared_ptr<Surface>& surface,
    uint32_t binding, uint32_t index) {
    if (index >= surface->ImageCount()) {
        return;
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkSampler sampler;
    vkCreateSampler(mVulkanManager->Device(), &samplerInfo, nullptr, &sampler);

    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = surface->Images()[index];
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = surface->Format();
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(mVulkanManager->Device(), &imageViewInfo,
        nullptr, &imageView));

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = surface->ImageLayouts()[index];
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
        nullptr);
}

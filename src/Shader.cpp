#include "Shader.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>

#include <spirv_cross/spirv_glsl.hpp>

shaderc_shader_kind getShaderKind(ShaderStage stage)
{
    switch (stage)
    {
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

VkShaderStageFlagBits getVulkanShaderStage(ShaderStage stage)
{
    switch (stage)
    {
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

std::vector<uint32_t> compileShader(const std::string& filename, shaderc_shader_kind kind)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    std::ifstream file(filename);
    if (!file.is_open())
    {
        return { };
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
        source,
        kind,
        filename.c_str(),
        options
    );

    if (module.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cerr << "Error compiling shader " << filename << ": " << module.GetErrorMessage() << std::endl;
        return { };
    }

    return std::vector<uint32_t>(module.cbegin(), module.cend());
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

Shader::Shader(const std::shared_ptr<VulkanManager>& vulkanManager, const std::string& filename, ShaderStage stage)
    : mVulkanManager(vulkanManager), mStage(getVulkanShaderStage(stage))
{
    auto spirv = compileShader(filename, getShaderKind(stage));
    mShader = createShaderModule(mVulkanManager->Device(), spirv);
}

Shader::~Shader()
{
    if (mShader != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(mVulkanManager->Device(), mShader, nullptr);
    }
}

VkPipelineShaderStageCreateInfo Shader::CreateShaderStageInfo() const
{
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = mStage;
    shaderStageInfo.module = mShader;
    shaderStageInfo.pName = "main"; // Entry point

    return shaderStageInfo;
}
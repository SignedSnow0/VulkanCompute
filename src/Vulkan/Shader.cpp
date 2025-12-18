#include "Shader.h"

#include <array>
#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <sstream>
#include <vector>

class NEShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    NEShaderIncluder(const std::string basePath) : mBasePath(basePath) {}

    shaderc_include_result *GetInclude(const char *requested_source,
                                       shaderc_include_type type,
                                       const char *requesting_source,
                                       size_t include_depth) {

        std::ifstream file(mBasePath + "/" + requested_source);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open include file: {}", requested_source);
            return nullptr;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string contents = buffer.str();
        file.close();

        auto container = new std::array<std::string, 2>;
        (*container)[0] = requested_source;
        (*container)[1] = contents;

        auto data = new shaderc_include_result;

        data->user_data = container;

        data->source_name = (*container)[0].data();
        data->source_name_length = (*container)[0].size();

        data->content = (*container)[1].data();
        data->content_length = (*container)[1].size();

        return data;
    };

    void ReleaseInclude(shaderc_include_result *data) override {
        delete static_cast<std::array<std::string, 2> *>(data->user_data);
        delete data;
    };

private:
    std::string mBasePath;
};

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

std::vector<uint32_t> compileShader(const std::string &filename,
                                    shaderc_shader_kind kind) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    std::string filePath = filename.substr(0, filename.find_last_of("/\\"));

    options.SetIncluder(std::make_unique<NEShaderIncluder>(filePath));

    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARNING("Failed to open shader file: {}", filename);
        return {};
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(source, kind, filename.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG_WARNING("Error compiling shader {}: {}", filename,
                    module.GetErrorMessage());
        return {};
    }

    return std::vector<uint32_t>(module.cbegin(), module.cend());
}

VkShaderModule createShaderModule(VkDevice device,
                                  const std::vector<uint32_t> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

void createDescriptorSet(VkDevice device, const std::vector<uint32_t> &bytecode,
                         VkShaderStageFlagBits stage, uint32_t setCount,
                         std::vector<VkDescriptorSetLayout> &outLayouts,
                         VkDescriptorPool &outPool,
                         std::vector<VkDescriptorSet> &outSets,
                         std::map<std::string, uint32_t> &outBindingMap) {
    spirv_cross::Compiler glslCompiler(bytecode);
    spirv_cross::ShaderResources resources =
        glslCompiler.get_shader_resources();

    LOG_DEBUG(
        "Shader reflection: found {} uniform buffers, {} storage buffers, "
        "{} sampled images, {} storage images.",
        resources.uniform_buffers.size(), resources.storage_buffers.size(),
        resources.sampled_images.size(), resources.storage_images.size());

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto &resource : resources.storage_buffers) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        auto resourceName = glslCompiler.get_name(resource.id);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);

        outBindingMap[resourceName] = binding;

        LOG_DEBUG("\tFound storage buffer resource: \'{}\' at binding {}",
                  resourceName, binding);
    }

    for (const auto &resource : resources.sampled_images) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        auto resourceName = glslCompiler.get_name(resource.id);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);

        outBindingMap[resourceName] = binding;

        LOG_DEBUG("\tFound sampled image resource: \'{}\' at binding {}",
                  resourceName, binding);
    }

    for (const auto &resource : resources.uniform_buffers) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        auto resourceName = glslCompiler.get_name(resource.id);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);

        outBindingMap[resourceName] = binding;

        LOG_DEBUG("\tFound uniform buffer resource: \'{}\' at binding {}",
                  resourceName, binding);
    }

    for (const auto &resource : resources.storage_images) {
        uint32_t binding =
            glslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        auto resourceName = glslCompiler.get_name(resource.id);

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        bindings.push_back(layoutBinding);

        outBindingMap[resourceName] = binding;

        LOG_DEBUG("\tFound storage image resource: \'{}\' at binding {}",
                  resourceName, binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    outLayouts.resize(setCount);
    for (auto &layout : outLayouts) {
        VK_CHECK(
            vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto &binding : bindings) {
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
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = outLayouts.data();

    outSets.resize(setCount);
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, outSets.data()));
}

Shader *Shader::Create(const std::shared_ptr<VulkanManager> &vulkanManager,
                       const std::string &filename, ShaderStage stage,
                       uint32_t setCount) {
    auto *shader = new Shader(vulkanManager, filename, stage, setCount);
    if (shader->mShader == VK_NULL_HANDLE) {
        return nullptr;
    }

    return shader;
}

Shader::Shader(const std::shared_ptr<VulkanManager> &vulkanManager,
               const std::string &filename, ShaderStage stage,
               uint32_t setCount)
    : mVulkanManager(vulkanManager), mStage(getVulkanShaderStage(stage)) {
    auto spirv = compileShader(filename, getShaderKind(stage));
    if (spirv.empty()) {
        return;
    }

    mShader = createShaderModule(mVulkanManager->Device(), spirv);

    createDescriptorSet(mVulkanManager->Device(), spirv, mStage, setCount,
                        mDescriptorSetLayouts, mDescriptorPool, mDescriptorSets,
                        mBindingMap);

    LOG_INFO("Created shader: {}", filename);
}

Shader::~Shader() {
    for (const auto &layout : mDescriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(mVulkanManager->Device(), layout, nullptr);
    }
    vkDestroyDescriptorPool(mVulkanManager->Device(), mDescriptorPool, nullptr);
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

void Shader::BindImage(const Image &image, uint32_t binding,
                       uint32_t frameIndex, bool isStorage) {
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = image.Layout();
    imageInfo.imageView = image.ImageView();
    imageInfo.sampler = image.Sampler();

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSets[frameIndex];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType =
        isStorage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
                           nullptr);
}

void Shader::BindSurfaceAsImage(const std::shared_ptr<Surface> &surface,
                                uint32_t binding, uint32_t index) {
    if (index >= surface->ImageCount()) {
        return;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = surface->ImageLayouts()[index];
    imageInfo.imageView = surface->ImageViews()[index];
    imageInfo.sampler = surface->Samplers()[index];

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSets[index];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(mVulkanManager->Device(), 1, &descriptorWrite, 0,
                           nullptr);
}

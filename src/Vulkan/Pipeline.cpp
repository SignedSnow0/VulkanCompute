#include "Pipeline.h"

VkPipelineLayout
createPipelineLayout(VkDevice device,
                     VkDescriptorSetLayout mDescriptorSetLayout) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                    &pipelineLayout));
    return pipelineLayout;
}

VkPipeline createGraphicsPipeline(
    VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
    const std::vector<std::shared_ptr<Shader>> &shaders, VkExtent2D extent) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (const auto &shader : shaders) {
        shaderStages.push_back(shader->CreateShaderStageInfo());
    }

    // Fixed-function stage configurations (placeholders, should be set
    // according to your needs)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;      // Optional
    vertexInputInfo.pVertexBindingDescriptions = nullptr;   // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;    // Optional
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = extent.width;
    viewport.height = extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0f;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional
    multisampling.pNext = nullptr;
    multisampling.flags = 0;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional
    pipelineInfo.layout = layout;
    pipelineInfo.pDynamicState = nullptr;      // Optional
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;

    VkPipeline graphicsPipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &graphicsPipeline));
    return graphicsPipeline;
}

VkPipeline createComputePipeline(VkDevice device, VkPipelineLayout layout,
                                 const std::shared_ptr<Shader> &computeShader) {
    VkPipelineShaderStageCreateInfo shaderStage =
        computeShader->CreateShaderStageInfo();

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = layout;
    pipelineInfo.flags = 0;                           // Optional
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional
    pipelineInfo.pNext = nullptr;

    VkPipeline computePipeline;
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                      nullptr, &computePipeline));
    return computePipeline;
}

Pipeline::Pipeline(const std::shared_ptr<VulkanManager> &vulkanManager,
                   const std::vector<std::shared_ptr<Shader>> &shaders,
                   const std::shared_ptr<RenderPass> &renderPass)
    : mVulkanManager(vulkanManager), mShaders(shaders),
      mRenderPass(renderPass) {
    mLayout = createPipelineLayout(mVulkanManager->Device(),
                                   mShaders[0]->DescriptorSetLayout(00));
    mPipeline = createGraphicsPipeline(mVulkanManager->Device(),
                                       mRenderPass->RenderPassHandle(), mLayout,
                                       mShaders, mRenderPass->Extent());
}

Pipeline::~Pipeline() {
    vkDestroyPipelineLayout(mVulkanManager->Device(), mLayout, nullptr);
    vkDestroyPipeline(mVulkanManager->Device(), mPipeline, nullptr);
}

void Pipeline::Bind(const std::shared_ptr<CommandBuffer> &commandBuffer) {
    commandBuffer->ExecuteCommand([this](VkCommandBuffer cmdBuffer) {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipeline);

        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
    });
}

ComputePipeline::ComputePipeline(
    const std::shared_ptr<VulkanManager> &vulkanManager,
    const std::shared_ptr<Shader> &computeShader)
    : mVulkanManager(vulkanManager), mComputeShader(computeShader) {
    mLayout = createPipelineLayout(mVulkanManager->Device(),
                                   mComputeShader->DescriptorSetLayout(0));
    mPipeline = createComputePipeline(mVulkanManager->Device(), mLayout,
                                      mComputeShader);
}

ComputePipeline::~ComputePipeline() {
    if (mLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(mVulkanManager->Device(), mLayout, nullptr);
    }
    if (mPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(mVulkanManager->Device(), mPipeline, nullptr);
    }
}

void ComputePipeline::Dispatch(
    const std::shared_ptr<CommandBuffer> &commandBuffer, uint32_t groupCountX,
    uint32_t groupCountY, uint32_t groupCountZ) {
    commandBuffer->ExecuteCommand([this, groupCountX, groupCountY, groupCountZ,
                                   commandBuffer](VkCommandBuffer cmdBuffer) {
        VkDescriptorSet descriptorSet =
            mComputeShader->DescriptorSet(commandBuffer->CurrentBufferIndex());
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                mLayout, 0, 1, &descriptorSet, 0, nullptr);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
        vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
    });
}

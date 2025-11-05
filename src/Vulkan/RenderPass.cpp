#include "RenderPass.h"

VkRenderPass createRenderPass(VkDevice device, VkFormat format) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

std::vector<VkFramebuffer>
createFramebuffers(VkDevice device, VkRenderPass renderPass,
                   const std::vector<VkImage> &images,
                   const std::vector<VkImageView> &imageViews,
                   VkExtent2D extent, VkFormat format) {
    std::vector<VkFramebuffer> framebuffers(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageViews[i];
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                     &framebuffers[i]));
    }

    return framebuffers;
}

RenderPass::RenderPass(const std::shared_ptr<VulkanManager> &vulkanManager,
                       const std::shared_ptr<Surface> &surface)
    : mVulkanManager(vulkanManager), mSurface(surface) {
    mRenderPass =
        createRenderPass(mVulkanManager->Device(), mSurface->Format());
    mFramebuffers = createFramebuffers(
        mVulkanManager->Device(), mRenderPass, mSurface->Images(),
        mSurface->ImageViews(), mSurface->Extent(), surface->Format());
}

RenderPass::~RenderPass() {
    for (auto framebuffer : mFramebuffers) {
        vkDestroyFramebuffer(mVulkanManager->Device(), framebuffer, nullptr);
    }

    if (mRenderPass) {
        vkDestroyRenderPass(mVulkanManager->Device(), mRenderPass, nullptr);
    }
}

void RenderPass::Begin(const std::shared_ptr<CommandBuffer> &commandBuffer,
                       bool clear) {
    if (commandBuffer->CurrentBufferIndex() >= mFramebuffers.size()) {
        return;
    }

    commandBuffer->ExecuteCommand([&](VkCommandBuffer cmdBuffer) {
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass;
        renderPassInfo.framebuffer =
            mFramebuffers[commandBuffer->CurrentBufferIndex()];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSurface->Extent();

        if (clear) {
            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
        } else {
            renderPassInfo.clearValueCount = 0;
            renderPassInfo.pClearValues = nullptr;
        }

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);
    });
}

void RenderPass::End(const std::shared_ptr<CommandBuffer> &commandBuffer) {
    commandBuffer->ExecuteCommand(
        [&](VkCommandBuffer cmdBuffer) { vkCmdEndRenderPass(cmdBuffer); });
}

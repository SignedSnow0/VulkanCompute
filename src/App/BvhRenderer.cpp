#include "BvhRenderer.h"

BvhRenderer::BvhRenderer(const std::shared_ptr<VulkanManager>& vulkanManager, const BvhBuilder& bvhBuilder)
    : mVulkanManager(vulkanManager) {
    const auto& bvhNodes = bvhBuilder.GetBvh();
    const auto& triangles = bvhBuilder.GetTriangles();
    
    mBvhBuffer = std::make_unique<Buffer<BvhNode>>(
        mVulkanManager, bvhNodes.data(), sizeof(BvhNode) * bvhNodes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    mTrianglesBuffer = std::make_unique<Buffer<Triangle>>(
        mVulkanManager, triangles.data(), sizeof(Triangle) * triangles.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void BvhRenderer::Render(const std::shared_ptr<CommandBuffer>& commandBuffer, const std::shared_ptr<Shader>& shader) const {
    shader->BindBuffer(*mTrianglesBuffer, "gTrianglesBuffer",
        commandBuffer->CurrentBufferIndex());

    shader->BindBuffer(*mBvhBuffer, "gBvhBuffer",
        commandBuffer->CurrentBufferIndex());
}
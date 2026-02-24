#pragma once

#include <vector>

#include "BvhBuilder.h"
#include "Core/AssetManager.h"
#include "Vulkan/Shader.h"

class BvhRenderer {
public:
    BvhRenderer(const std::shared_ptr<VulkanManager>& vulkanManager, const BvhBuilder& bvhBuilder);

    void Render(const std::shared_ptr<CommandBuffer>& commandBuffer, const std::shared_ptr<Shader>& shader) const;

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    
    std::unique_ptr<Buffer<BvhNode>> mBvhBuffer;
    std::unique_ptr<Buffer<Triangle>> mTrianglesBuffer;
};
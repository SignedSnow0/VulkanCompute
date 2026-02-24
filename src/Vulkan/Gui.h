#pragma once

#include <memory>
#include <imgui.h>

#include "Vulkan/VulkanManager.h"
#include "Vulkan/Surface.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Image.h"

class Gui {
public:
    Gui(const std::shared_ptr<VulkanManager>& vulkanManager, const std::shared_ptr<Surface>& surface, const Window& window);
    ~Gui();

    void Begin(const std::shared_ptr<CommandBuffer>& commandBuffer);
    void End(const std::shared_ptr<CommandBuffer>& commandBuffer);

    ImTextureID RegisterImage(const std::shared_ptr<Image>& image) const;

private:
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Surface> mSurface;

    std::unique_ptr<RenderPass> mRenderPass;
    VkDescriptorPool mDescriptorPool;
};
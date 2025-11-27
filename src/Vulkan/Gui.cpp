#include "Gui.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

VkDescriptorPool createDescriptorPool(const std::shared_ptr<VulkanManager> &manager) {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(manager->Device(), &poolInfo, nullptr, &descriptorPool));

    return descriptorPool;
}

void initGui(const std::shared_ptr<VulkanManager>& manager, Window& window, const std::shared_ptr<Surface>& surface, const std::shared_ptr<RenderPass> &renderPass, VkDescriptorPool descriptorPool) {
    ImGui::CreateContext();
    ImGuiPlatformIO& io = ImGui::GetPlatformIO();

    ImGui_ImplGlfw_InitForVulkan(window.Handle(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = manager->Instance();
    initInfo.PhysicalDevice = manager->PhysicalDevice();
    initInfo.Device = manager->Device();
    initInfo.QueueFamily = manager->ComputeQueueFamilyIndex();
    initInfo.Queue = manager->ComputeQueue();
    initInfo.MinImageCount = surface->ImageCount();
    initInfo.ImageCount = surface->ImageCount();
    initInfo.DescriptorPool = descriptorPool;
    initInfo.PipelineInfoMain.RenderPass = renderPass->RenderPassHandle();
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}

Gui::Gui(const std::shared_ptr<VulkanManager>& manager, Window& window, const std::shared_ptr<Surface>& surface)
    : mVulkanManager(manager), mSurface(surface) {
    mDescriptorPool = createDescriptorPool(manager);

    mRenderPass = std::make_shared<RenderPass>(mVulkanManager, mSurface);

    initGui(manager, window, surface, mRenderPass, mDescriptorPool);
}

Gui::~Gui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    vkDestroyDescriptorPool(mVulkanManager->Device(), mDescriptorPool, nullptr);
}

void Gui::Begin(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::End(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    ImGui::Render();
}

void Gui::Submit(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    mRenderPass->Begin(commandBuffer);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->CurrentBuffer());
    mRenderPass->End(commandBuffer);
}
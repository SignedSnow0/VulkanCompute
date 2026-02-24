#include "Gui.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

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

void initImGui(const std::shared_ptr<VulkanManager>& vulkanManager,
               const std::shared_ptr<Surface>& surface,
               const std::unique_ptr<RenderPass>& renderPass,
               GLFWwindow* window,
               VkDescriptorPool descriptorPool) {
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance       = vulkanManager->Instance();
    initInfo.PhysicalDevice = vulkanManager->PhysicalDevice();
    initInfo.Device         = vulkanManager->Device();
    initInfo.QueueFamily    = vulkanManager->GraphicsQueue().familyIndex;
    initInfo.Queue          = vulkanManager->GraphicsQueue().queue;
    initInfo.PipelineCache  = VK_NULL_HANDLE;
    initInfo.DescriptorPool = descriptorPool;
    initInfo.MinImageCount  = surface->ImageCount();
    initInfo.ImageCount     = surface->ImageCount();
    initInfo.Allocator      = nullptr;
    initInfo.CheckVkResultFn = nullptr;
    initInfo.PipelineInfoMain.RenderPass = renderPass->RenderPassHandle();
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
}

Gui::Gui(const std::shared_ptr<VulkanManager>& vulkanManager, const std::shared_ptr<Surface>& surface, const Window& window)
    : mVulkanManager(vulkanManager), mSurface(surface) {
    mRenderPass = std::make_unique<RenderPass>(mVulkanManager, mSurface);
        
    mDescriptorPool = createDescriptorPool(mVulkanManager);
    initImGui(mVulkanManager, mSurface, mRenderPass, window.Handle(), mDescriptorPool);
}

Gui::~Gui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(mVulkanManager->Device(), mDescriptorPool, nullptr);
}

void Gui::Begin(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    mRenderPass->Begin(commandBuffer, true);
}

void Gui::End(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    ImGui::Render();
   
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

     ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->CurrentBuffer());

    mRenderPass->End(commandBuffer);

}

ImTextureID Gui::RegisterImage(const std::shared_ptr<Image>& image) const {
    return reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(image->Sampler(), image->ImageView(), image->Layout()));
}
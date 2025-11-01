#include <memory>

#include "CommandBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Surface.h"
#include "VulkanManager.h"
#include "Window.h"

int main(int argc, char **argv) {
    Window window(1280, 720, "Vulkan Compute");

    auto vulkanManager = std::make_shared<VulkanManager>(window);
    auto surface = std::make_shared<Surface>(vulkanManager, window);
    auto renderPass = std::make_shared<RenderPass>(vulkanManager, surface);

    auto vertex = std::make_shared<Shader>(
        vulkanManager, "assets/shaders/default.vert.glsl", ShaderStage::Vertex,
        surface->ImageCount());
    auto fragment = std::make_shared<Shader>(
        vulkanManager, "assets/shaders/default.frag.glsl",
        ShaderStage::Fragment, surface->ImageCount());
    std::vector<std::shared_ptr<Shader>> shaders = {vertex, fragment};

    auto pipeline =
        std::make_shared<Pipeline>(vulkanManager, shaders, renderPass);

    auto computeShader = std::make_shared<Shader>(
        vulkanManager, "assets/shaders/RayTracer.comp.glsl",
        ShaderStage::Compute, surface->ImageCount());
    auto computePipeline =
        std::make_shared<ComputePipeline>(vulkanManager, computeShader);

    auto commandBuffer =
        std::make_shared<CommandBuffer>(vulkanManager, surface->ImageCount());

    while (!window.shouldClose()) {
        window.pollEvents();

        uint32_t imageIndex = surface->WaitNextImage();
        commandBuffer->Begin(imageIndex);

        surface->ChangeLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL);

        computeShader->BindSurfaceAsImage(surface, 0, imageIndex);
        computePipeline->Dispatch(commandBuffer,
                                  (surface->Extent().width + 15) / 16,
                                  (surface->Extent().height + 15) / 16, 1);

        uint32_t submitIndex = commandBuffer->End();
        surface->ChangeLayout(commandBuffer,
                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        surface->SubmitCommandBuffer(commandBuffer, submitIndex);
    }

    return 0;
}

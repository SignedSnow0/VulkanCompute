#include <memory>

#include "VulkanManager.h"
#include "Window.h"
#include "Surface.h"
#include "Shader.h"
#include "CommandBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"

int main(int argc, char** argv)
{
    Window window(1280, 720, "Vulkan Compute");

    auto vulkanManager = std::make_shared<VulkanManager>(window);
    auto surface = std::make_shared<Surface>(vulkanManager, window);
    auto renderPass = std::make_shared<RenderPass>(vulkanManager, surface);

    auto vertex = std::make_shared<Shader>(vulkanManager, "assets/shaders/vert.glsl", ShaderStage::Vertex);
    auto fragment = std::make_shared<Shader>(vulkanManager, "assets/shaders/frag.glsl", ShaderStage::Fragment);
    std::vector<std::shared_ptr<Shader>> shaders = { vertex, fragment };

    auto pipeline = std::make_shared<Pipeline>(vulkanManager, shaders, renderPass);
    auto commandBuffer = std::make_shared<CommandBuffer>(vulkanManager, surface->ImageCount());

    while (!window.shouldClose()) {
        window.pollEvents();

        uint32_t imageIndex = surface->WaitNextImage();
        commandBuffer->Begin(imageIndex);
        renderPass->Begin(commandBuffer, imageIndex);

        pipeline->Bind(commandBuffer);

        renderPass->End(commandBuffer);
        uint32_t submitIndex = commandBuffer->End();
        surface->SubmitCommandBuffer(commandBuffer, submitIndex);
    }

    return 0;
}
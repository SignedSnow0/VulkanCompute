#pragma once

#include <memory>

#include "Core/Window.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Surface.h"
#include "Vulkan/VulkanManager.h"

/**
 * @brief Base class for Vulkan compute applications.
 *
 * This class provides a framework for creating Vulkan compute applications by
 * managing common necessary resources.
 * Derived classes should implement the virtual methods to define application
 * behavior.
 */
class VulkanComputeApp {
public:
    VulkanComputeApp(uint32_t windowWidth, uint32_t windowHeight,
                     const char *windowTitle);
    virtual ~VulkanComputeApp();

    /**
     * @brief Called once at the start of the application.
     *
     * The method should be used to initialize static resources.
     */
    virtual void OnStart() = 0;
    /**
     * @brief Called once per frame to update application logic.
     *
     * This method is called at the beginning of each frame, before rendering
     * operations.
     * @see OnRender
     *
     * @param dt Time elapsed since the last frame in seconds.
     */
    virtual void OnUpdate(float dt) = 0;
    /**
     * @brief Called once per frame to record rendering commands.
     *
     * This method is called after OnUpdate and should be used to record
     * commands for rendering.
     * @see OnUpdate
     *
     * @param dt Time elapsed since the last frame in seconds.
     * @param commandBuffer The command buffer to record rendering commands
     * into.
     */
    virtual void OnRender(float dt,
                          std::shared_ptr<CommandBuffer> commandBuffer) = 0;
    /**
     * @brief Called once at the end of the application.
     *
     * This method should be used to release resources and perform any necessary
     * cleanup.
     */
    virtual void OnStop() = 0;

protected:
    Window mWindow;
    std::shared_ptr<VulkanManager> mVulkanManager;
    std::shared_ptr<Surface> mSurface;

private:
    void Start();
    void MainLoop();
    void Stop();

    std::shared_ptr<RenderPass> mRenderPass;
    std::shared_ptr<ComputePipeline> mComputePipeline;
    std::shared_ptr<CommandBuffer> mCommandBuffer;

    friend int main(int argc, char **argv);
};
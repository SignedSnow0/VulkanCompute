#include "Surface.h"

#include <GLFW/glfw3native.h>

#include "Vulkan/Utils.h"

VkSurfaceKHR createSurface(GLFWwindow *window, VkInstance instance) {
    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    return surface;
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device,
                               VkSurfaceKHR surface, VkFormat format,
                               VkExtent2D extent, uint32_t preferredImageCount = 0) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                              &surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                         nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                         surfaceFormats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                              &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR || availablePresentMode == VK_PRESENT_MODE_FIFO_LATEST_READY_KHR) {
            presentMode = availablePresentMode;
            break;
        }
    }

    uint32_t imageCount = surfaceCapabilities.minImageCount;
    if (surfaceCapabilities.maxImageCount > 0 &&
        imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }
    if (preferredImageCount && preferredImageCount >= surfaceCapabilities.minImageCount &&
        (surfaceCapabilities.maxImageCount == 0 ||
         preferredImageCount <= surfaceCapabilities.maxImageCount)) {
        imageCount = preferredImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    VkSwapchainKHR swapchain;
    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));
    return swapchain;
}

std::vector<VkImage> getSwapChainImages(VkDevice device,
                                        VkSwapchainKHR swapchain) {
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

    return images;
}

std::vector<VkImageView> createImageViews(VkDevice device,
                                          const std::vector<VkImage> &images,
                                          VkFormat format) {
    std::vector<VkImageView> imageViews;
    imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK(
            vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]));
    }

    return imageViews;
}

void createSyncObjects(VkDevice device,
                       std::vector<VkSemaphore> &imageAvailable,
                       std::vector<VkSemaphore> &renderFinished,
                       std::vector<VkFence> &inFlight, uint32_t count) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    imageAvailable.resize(count);
    renderFinished.resize(count);
    inFlight.resize(count);

    for (uint32_t i = 0; i < count; i++) {
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                                   &imageAvailable[i]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                                   &renderFinished[i]));
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &inFlight[i]));
    }
}

std::vector<VkSampler> CreateSamplers(VkDevice device, uint32_t count) {
    std::vector<VkSampler> samplers;
    samplers.resize(count);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    for (uint32_t i = 0; i < count; i++) {
        vkCreateSampler(device, &samplerInfo, nullptr, &samplers[i]);
    }
    return samplers;
}

Surface::Surface(const std::shared_ptr<VulkanManager> &vulkanManager,
                 const Window &window, VkImageLayout initialLayout, uint32_t imageCount)
    : mVulkanManager(vulkanManager) {
    mFormat = VK_FORMAT_R8G8B8A8_UNORM;
    mExtent = {window.Width(), window.Height()};

    mSurface = createSurface(window.Handle(), mVulkanManager->Instance());

    mSwapchain =
        createSwapChain(mVulkanManager->PhysicalDevice(),
                        mVulkanManager->Device(), mSurface, mFormat, mExtent, imageCount);
    mSwapchainImages = getSwapChainImages(mVulkanManager->Device(), mSwapchain);
    mSwapchainImageViews =
        createImageViews(mVulkanManager->Device(), mSwapchainImages, mFormat);
    mSamplers =
        CreateSamplers(mVulkanManager->Device(), mSwapchainImages.size());

    createSyncObjects(mVulkanManager->Device(), mImageAvailableSemaphores,
                      mRenderFinishedSemaphores, mInFlightFences,
                      mSwapchainImages.size());

    mLayouts = std::vector<VkImageLayout>(mSwapchainImages.size(),
                                          VK_IMAGE_LAYOUT_UNDEFINED);

    vulkanManager->SubmitCommand(
        [this, initialLayout](VkCommandBuffer cmdBuffer) {
            for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
                changeLayout(cmdBuffer, mLayouts[i], initialLayout,
                             mSwapchainImages[i]);
                mLayouts[i] = initialLayout;
            }
        });

    LOG_INFO("Swapchain created with {} images.", mSwapchainImages.size());
}

Surface::~Surface() {
    for (uint32_t i = 0; i < mImageAvailableSemaphores.size(); i++) {
        vkDestroySampler(mVulkanManager->Device(), mSamplers[i], nullptr);
        vkDestroyImageView(mVulkanManager->Device(), mSwapchainImageViews[i],
                           nullptr);
        vkDestroySemaphore(mVulkanManager->Device(),
                           mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mVulkanManager->Device(),
                           mRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(mVulkanManager->Device(), mInFlightFences[i], nullptr);
    }
    vkDestroySwapchainKHR(mVulkanManager->Device(), mSwapchain, nullptr);
    vkDestroySurfaceKHR(mVulkanManager->Instance(), mSurface, nullptr);
}

uint32_t Surface::WaitNextImage() {
    vkWaitForFences(mVulkanManager->Device(), 1,
                    &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(mVulkanManager->Device(), 1, &mInFlightFences[mCurrentFrame]);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mVulkanManager->Device(), mSwapchain, UINT64_MAX,
                          mImageAvailableSemaphores[mCurrentFrame],
                          VK_NULL_HANDLE, &imageIndex);
    return imageIndex;
}

void Surface::SubmitCommandBuffer(
    const std::shared_ptr<CommandBuffer> &commandBuffer,
    uint32_t commandBufferIndex) {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkCommandBuffer cmdBuffer = commandBuffer->Buffers()[commandBufferIndex];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VK_CHECK(vkQueueSubmit(mVulkanManager->ComputeQueue(), 1, &submitInfo,
                           mInFlightFences[mCurrentFrame]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mSwapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &commandBufferIndex;
    presentInfo.pResults = nullptr; // Optional

    VK_CHECK(vkQueuePresentKHR(mVulkanManager->ComputeQueue(), &presentInfo));

    mCurrentFrame = (mCurrentFrame + 1) % mSwapchainImages.size();
}

void Surface::ChangeLayout(const std::shared_ptr<CommandBuffer> &commandBuffer,
                           VkImageLayout newLayout) {
    uint32_t index = commandBuffer->CurrentBufferIndex();
    auto cmdBuffer = commandBuffer->CurrentBuffer();

    changeLayout(cmdBuffer, mLayouts[index], newLayout,
                 mSwapchainImages[index]);
    mLayouts[index] = newLayout;
}
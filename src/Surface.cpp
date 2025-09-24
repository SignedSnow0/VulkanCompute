#include "Surface.h"

#include <GLFW/glfw3native.h>

VkSurfaceKHR createSurface(GLFWwindow *window, VkInstance instance)
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        return nullptr;
    }
    
    return surface;
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device,
                               VkSurfaceKHR surface, VkFormat format, VkExtent2D extent)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

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
    for (const auto &availablePresentMode : presentModes) 
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = availablePresentMode;
            break;
        }
    }
  
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 &&
        imageCount > surfaceCapabilities.maxImageCount) 
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }
  
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
  
    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) 
    {
        return nullptr;
    }

    return swapchain;
}

std::vector<VkImage> getSwapChainImages(VkDevice device,
                                        VkSwapchainKHR swapchain)
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
    
    return images;
}

std::vector<VkImageView> createImageViews(VkDevice device,
                                        const std::vector<VkImage> &images,
                                        VkFormat format)
{
    std::vector<VkImageView> imageViews;
    imageViews.resize(images.size());
  
    for (size_t i = 0; i < images.size(); i++) 
    {
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
    
        if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]) !=
            VK_SUCCESS)
        {
            return { };
        }
    }
  
    return imageViews;
}

VkSemaphore createSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  
    VkSemaphore semaphore;
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) !=
        VK_SUCCESS)
    {
        return nullptr;
    }
  
    return semaphore;
}

VkFence createFence(VkDevice device, bool signaled)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
  
    VkFence fence;
    if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) 
    {
        return nullptr;
    }
  
    return fence;
}

Surface::Surface(const std::shared_ptr<VulkanManager> &vulkanManager,
                 const Window &window)
    : mVulkanManager(vulkanManager)
{
    mFormat = VK_FORMAT_B8G8R8A8_SRGB;
    mExtent = { window.Width(), window.Height() };
  
    mSurface = createSurface(window.Handle(), mVulkanManager->Instance());
    
    mSwapchain = createSwapChain(mVulkanManager->PhysicalDevice(),
                                 mVulkanManager->Device(), mSurface, mFormat, mExtent);
    mSwapchainImages = getSwapChainImages(mVulkanManager->Device(), mSwapchain);
    mSwapchainImageViews = createImageViews(mVulkanManager->Device(), mSwapchainImages, VK_FORMAT_B8G8R8A8_SRGB);

    mSemaphore = createSemaphore(mVulkanManager->Device());
    mFence = createFence(mVulkanManager->Device(), true);
}

Surface::~Surface() 
{
    if (mFence) 
    {
        vkDestroyFence(mVulkanManager->Device(), mFence, nullptr);
    }

    if (mSemaphore) 
    {
        vkDestroySemaphore(mVulkanManager->Device(), mSemaphore, nullptr);
    }

    if (mSwapchain)
    {
        vkDestroySwapchainKHR(mVulkanManager->Device(), mSwapchain, nullptr);
    }

    if (mSurface) 
    {
        vkDestroySurfaceKHR(mVulkanManager->Instance(), mSurface, nullptr);
    }
}

uint32_t Surface::WaitNextImage()
{
    vkWaitForFences(mVulkanManager->Device(), 1, &mFence, VK_TRUE, UINT64_MAX);
    vkResetFences(mVulkanManager->Device(), 1, &mFence);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mVulkanManager->Device(), mSwapchain, UINT64_MAX, mSemaphore, VK_NULL_HANDLE, &imageIndex);
    return imageIndex;
}

void Surface::SubmitCommandBuffer(const std::shared_ptr<CommandBuffer>& commandBuffer, uint32_t commandBufferIndex)
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { mSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkSemaphore signalSemaphores[] = { mSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkCommandBuffer cmdBuffer = commandBuffer->Buffers()[commandBufferIndex];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    if (vkQueueSubmit(mVulkanManager->ComputeQueue(), 1, &submitInfo, mFence) != VK_SUCCESS)
    {
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &commandBufferIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(mVulkanManager->ComputeQueue(), &presentInfo);
}
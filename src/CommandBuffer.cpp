#include "CommandBuffer.h"

#include <iostream>

VkCommandPool createCommandPool(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        return nullptr;
    }

    return commandPool;
}

std::vector<VkCommandBuffer> allocateCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t count)
{
    std::vector<VkCommandBuffer> commandBuffers(count);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        return {};
    }

    return commandBuffers;
}

CommandBuffer::CommandBuffer(const std::shared_ptr<VulkanManager>& vulkanManager, uint32_t count)
    : mVulkanManager(vulkanManager)
{
    mPool = createCommandPool(mVulkanManager->Device(), mVulkanManager->ComputeQueueFamilyIndex());
    mCommandBuffers = allocateCommandBuffers(mVulkanManager->Device(), mPool, count);
    mCurrentBufferIndex = std::nullopt;
}

CommandBuffer::~CommandBuffer()
{
    if (mPool) 
    {
        vkDestroyCommandPool(mVulkanManager->Device(), mPool, nullptr);
        mPool = nullptr;
    }
}

VkCommandBuffer CommandBuffer::CurrentBuffer() const
{
    if (!mCurrentBufferIndex)
    {
        std::cerr << "No command buffer is currently being recorded." << std::endl;
        return nullptr;
    }

    return mCommandBuffers[mCurrentBufferIndex.value()];
}

uint32_t CommandBuffer::CurrentBufferIndex() const
{
    if (!mCurrentBufferIndex)
    {
        return -1;
    }

    return mCurrentBufferIndex.value();
}

void CommandBuffer::Begin(uint32_t index)
{
    if (index >= mCommandBuffers.size() || mCurrentBufferIndex)
    {
        return;
    }

    VkCommandBuffer commandBuffer = mCommandBuffers[index];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResetCommandBuffer(commandBuffer, 0);
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        return;
    }

    mCurrentBufferIndex = index;
}

uint32_t CommandBuffer::End()
{
    if (!mCurrentBufferIndex)
    {
        return -1;
    }

    VkCommandBuffer commandBuffer = mCommandBuffers[mCurrentBufferIndex.value()];

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        return -1;
    }

    uint32_t index = mCurrentBufferIndex.value();
    mCurrentBufferIndex = std::nullopt;
    return index;
}

void CommandBuffer::ExecuteCommand(std::function<void(VkCommandBuffer commandBuffer)> func)
{
    if (!mCurrentBufferIndex)
    {
        return;
    }

    func(mCommandBuffers[mCurrentBufferIndex.value()]);
}
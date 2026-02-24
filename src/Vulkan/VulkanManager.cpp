#include "VulkanManager.h"

#include <format>
#include <string.h>
#include <vector>
#include <vulkan/vulkan_core.h>


#include "Core/Logger.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_DEBUG("[Validation layer]: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INFO("[Validation layer]: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARNING("[Validation layer]: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR("[Validation layer]: {}", pCallbackData->pMessage);
        break;
    default:
        break;
    }

    return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

bool instanceLayersSupported(const std::vector<const char *> &layers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : layers) {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            LOG_WARNING("Required instance layer \"{}\" not found", layerName);
            return false;
        }
    }
    return true;
}

bool deviceExtensionsSupported(VkPhysicalDevice device,
                               const std::vector<const char *> &extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    for (const char *extensionName : extensions) {
        bool extensionFound = false;
        for (const auto &extensionProperties : availableExtensions) {
            if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            return false;
        }
    }
    return true;
}

VkInstance createInstance(const std::vector<const char *> &requiredLayers,
                          const std::vector<const char *> &requiredExtensions) {
    if (!instanceLayersSupported(requiredLayers)) {
        LOG_ERROR("Required instance layers are not supported");
        return nullptr;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Compute App";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Vulkan Compute";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
    return instance;
}

#ifndef NDEBUG
VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    VkDebugUtilsMessengerEXT debugMessenger;
    VK_CHECK(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                          &debugMessenger));
    return debugMessenger;
}
#endif

VkPhysicalDevice choosePhysicalDevice(VkInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        return nullptr;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if (deviceProperties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            deviceProperties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            LOG_INFO("Selected discrete/integrated GPU: {}",
                     deviceProperties.deviceName);
            return device;
        }
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(devices[0], &deviceProperties);

    LOG_INFO("Selected default GPU: {}", deviceProperties.deviceName);

    return devices[0];
}

VkDevice createDevice(VkPhysicalDevice physicalDevice,
                      const std::vector<const char *> &requiredLayers,
    const std::vector<const char*>& requiredExtensions,
    Queue& graphicsQueue, Queue& computeQueue) {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
        nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
        queueFamilies.data());

    uint32_t graphicsQueueFamilyIndex = -1;
    uint32_t computeQueueFamilyIndex = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            computeQueueFamilyIndex = i;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = i;
        }

        if (graphicsQueueFamilyIndex != -1 && computeQueueFamilyIndex != -1) {
            break;
        }
    }

    if (computeQueueFamilyIndex == -1 || graphicsQueueFamilyIndex == -1) {
        LOG_ERROR("Failed to find a compute or graphics queue family");
        return nullptr;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    if (graphicsQueueFamilyIndex == computeQueueFamilyIndex) {
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    } else {
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueCreateInfos.push_back(queueCreateInfo);

        queueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();

    VkDevice device;
    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

    graphicsQueue.familyIndex = graphicsQueueFamilyIndex;
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue.queue);

    computeQueue.familyIndex = computeQueueFamilyIndex;
    vkGetDeviceQueue(device, computeQueueFamilyIndex, 0, &computeQueue.queue);

    return device;
}

static VkCommandPool createCommandPool(VkDevice device,
                                       uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool commandPool;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
    return commandPool;
}

VkCommandBuffer createCommandBuffer(VkDevice device,
                                    VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));
    return commandBuffer;
}

VulkanManager::VulkanManager(Window &window) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> instanceExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifndef NDEBUG
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<const char *> layers = {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    mInstance = createInstance(layers, instanceExtensions);
#ifndef NDEBUG
    mDebugMessenger = createDebugMessenger(mInstance);
#endif
    mPhysicalDevice = choosePhysicalDevice(mInstance);
    mDevice = createDevice(mPhysicalDevice, layers, deviceExtensions, mGraphicsQueue, mComputeQueue);

    mCommandPool = createCommandPool(mDevice, 0);
    mCommandBuffer = createCommandBuffer(mDevice, mCommandPool);

    LOG_INFO("VulkanManager initialized successfully");
}

VulkanManager::~VulkanManager() {
    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mCommandBuffer);
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyDevice(mDevice, nullptr);

#ifndef NDEBUG
    destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

    vkDestroyInstance(mInstance, nullptr);
}

void VulkanManager::SubmitCommand(std::function<void(VkCommandBuffer)> func, bool graphics) {
    vkResetCommandPool(mDevice, mCommandPool, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(mCommandBuffer, &beginInfo);

    func(mCommandBuffer);

    vkEndCommandBuffer(mCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffer;

    vkQueueSubmit(graphics ? mGraphicsQueue.queue : mComputeQueue.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics ? mGraphicsQueue.queue : mComputeQueue.queue);
}

void VulkanManager::WaitIdle() const { vkDeviceWaitIdle(mDevice); }

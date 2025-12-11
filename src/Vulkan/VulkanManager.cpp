#include "VulkanManager.h"

#include <format>
#include <string.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Core/Logger.h"
#include <set>

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
                      const std::vector<const char *> &requiredExtensions) {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex =
        0; // Assume family index 0 supports compute
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();

    VkDevice device;
    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));
    return device;
}

QueueInfo getQueues(VkDevice device, VkPhysicalDevice physicalDevice) {
    QueueInfo queueInfo{};
    queueInfo.graphics = VK_NULL_HANDLE;
    queueInfo.transfer = VK_NULL_HANDLE;
    queueInfo.compute = VK_NULL_HANDLE;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
        &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueInfo.graphics == VK_NULL_HANDLE &&
            (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            vkGetDeviceQueue(device, i, 0, &queueInfo.graphics);
            queueInfo.graphicsFamilyIndex = i;
        }

        if (queueInfo.transfer == VK_NULL_HANDLE &&
            (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            vkGetDeviceQueue(device, i, 0, &queueInfo.transfer);
            queueInfo.transferFamilyIndex = i;
        }

        if (queueInfo.compute == VK_NULL_HANDLE &&
             (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            vkGetDeviceQueue(device, i, 0, &queueInfo.compute);
            queueInfo.computeFamilyIndex = i;
        }

        if (queueInfo.graphics != VK_NULL_HANDLE &&
            queueInfo.transfer != VK_NULL_HANDLE &&
            queueInfo.compute != VK_NULL_HANDLE) {
            break;
        }
    }

    return queueInfo;
}

static void createCommandPool(VkDevice device, QueueInfo &queueInfo) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    poolInfo.queueFamilyIndex = queueInfo.computeFamilyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &queueInfo.computeCommandPool));

    poolInfo.queueFamilyIndex = queueInfo.graphicsFamilyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &queueInfo.graphicsCommandPool));

    poolInfo.queueFamilyIndex = queueInfo.transferFamilyIndex;
    VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &queueInfo.transferCommandPool));
}

void createCommandBuffer(VkDevice device,
                                    QueueInfo &queueInfo) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    allocInfo.commandPool = queueInfo.computeCommandPool;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &queueInfo.computeCommandBuffer));

    allocInfo.commandPool = queueInfo.graphicsCommandPool;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &queueInfo.graphicsCommandBuffer));

    allocInfo.commandPool = queueInfo.transferCommandPool;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &queueInfo.transferCommandBuffer));
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
    mDevice = createDevice(mPhysicalDevice, layers, deviceExtensions);
    mQueues = getQueues(mDevice, mPhysicalDevice);

    createCommandPool(mDevice, mQueues);
    createCommandBuffer(mDevice, mQueues);

    LOG_INFO("VulkanManager initialized successfully");
}

VulkanManager::~VulkanManager() {
    vkDestroyDevice(mDevice, nullptr);

#ifndef NDEBUG
    destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif

    vkDestroyInstance(mInstance, nullptr);
}

static void submitCommand(std::function<void(VkCommandBuffer)> func,
                          VkDevice device, VkQueue queue,
                          VkCommandPool commandPool,
                          VkCommandBuffer commandBuffer) {
    vkResetCommandPool(device, commandPool, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    func(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

void VulkanManager::SubmitCommand(std::function<void(VkCommandBuffer)> func, CommandType type) {
    if (type == CommandType::Compute) {
        submitCommand(func, mDevice, mQueues.compute,
                      mQueues.computeCommandPool,
                      mQueues.computeCommandBuffer);
        return;
    }

    if (type == CommandType::Graphics) {
        submitCommand(func, mDevice, mQueues.graphics,
                      mQueues.graphicsCommandPool,
                      mQueues.graphicsCommandBuffer);
        return;
    }

    if (type == CommandType::Transfer) {
        submitCommand(func, mDevice, mQueues.transfer,
                      mQueues.transferCommandPool,
                      mQueues.transferCommandBuffer);
        return;
    }
}

void VulkanManager::WaitIdle() const { vkDeviceWaitIdle(mDevice); }

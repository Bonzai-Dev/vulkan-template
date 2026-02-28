#include <core/application/logger.hpp>
#include "device.hpp"
#include "volk.h"
#include "queue.hpp"
#include "vulkan.hpp"

namespace Core::Renderer {
  VulkanDevice::VulkanDevice() {

  }

  VkResult VulkanDevice::createInstance(
    const std::vector<const char *> &extensions,
    const std::vector<const char *> &layers
  ) {
    validationLayersEnabled = Vulkan::validationLayersSupported() && Vulkan::debugEnabled;

    constexpr VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Vulkan Application",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Vulkan engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 3, 0),
    };

    VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()
    };

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    if (validationLayersEnabled) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
      createInfo.ppEnabledLayerNames = layers.data();

      debugInfo = createDebugInfo();
      createInfo.pNext = &debugInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
      LOG_CORE_TRACE("Vulkan validation layers requested, but not available.");
    }

    if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
      LOG_CORE_ERROR("Failed to create Vulkan instance.");
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    volkLoadInstance(vulkanInstance);

    if (validationLayersEnabled)
      createDebugUtilsMessenger(vulkanInstance, createDebugInfo(), nullptr, &debugMessenger);

    return VK_SUCCESS;
  }

  void VulkanDevice::destroy() const {
    if (validationLayersEnabled)
      deleteDebugUtilsMessenger(vulkanInstance, nullptr, &debugMessenger);

    vkDestroyInstance(vulkanInstance, nullptr);
  }

  VkResult VulkanDevice::createDebugUtilsMessenger(
    const VkInstance &instance,
    const VkDebugUtilsMessengerCreateInfoEXT &createInfo,
    const VkAllocationCallbacks *allocator,
    VkDebugUtilsMessengerEXT *debugMessenger
  ) {
    static const auto vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (vkCreateDebugUtilsMessenger != nullptr)
      return vkCreateDebugUtilsMessenger(instance, &createInfo, nullptr, debugMessenger);

    LOG_CORE_ERROR("Failed to attach debugger. \"vkCreateDebugUtilsMessengerEXT\" extension not present.");
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  void VulkanDevice::deleteDebugUtilsMessenger(
    const VkInstance &instance,
    const VkAllocationCallbacks *allocator,
    const VkDebugUtilsMessengerEXT *debugMessenger
  ) {
    static const auto vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (vkDestroyDebugUtilsMessenger != nullptr)
      vkDestroyDebugUtilsMessenger(instance, *debugMessenger, nullptr);
  }

  VkDebugUtilsMessengerCreateInfoEXT VulkanDevice::createDebugInfo() {
    return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,

      // void VulkanDevice::createLogicalDevice() {
      // uint32_t queueFamilyCount = 0;
      // }
      .pNext = nullptr,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &Vulkan::debugCallback,
      .pUserData = nullptr
    };
  }

  void VulkanDevice::createPhysicalDevice(std::uint32_t deviceId) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      LOG_CORE_CRITICAL("Cannot find any GPUs with Vulkan support.");
      return;
    }

    if (deviceId >= deviceCount) {
      LOG_CORE_WARNING("Requested device index {} but there's only {} devices.", deviceId, deviceCount);
      deviceId = 0;
    }

    LOG_CORE_INFO("Selecting GPU {}", deviceId);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());
    physicalDevice = devices[deviceId];

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    if (!deviceFeatures.geometryShader)
      supportedStages ^= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if (!deviceFeatures.tessellationShader)
      supportedStages ^= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
  }

  void VulkanDevice::createLogicalDevice(
    std::vector<const char*> &extensions,
    std::uint32_t maxComputeQueues,
    std::uint32_t maxTransferQueues
  ) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
    fillQueueCreationInfo( maxComputeQueues, maxTransferQueues, queueCreateInfo);

    std::vector<std::vector<float>> queuePriorities;
    queuePriorities.resize( queueCreateInfo.size() );

    for( size_t i = 0u; i < queueCreateInfo.size(); ++i )
    {
      queuePriorities[i].resize( queueCreateInfo[i].queueCount, 1.0f );
      queueCreateInfo[i].pQueuePriorities = queuePriorities[i].data();
    }

    // extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

  }

  void VulkanDevice::fillQueueCreationInfo(
    std::uint32_t maxComputeQueue, std::uint32_t maxTransferQueue,
    std::vector<VkDeviceQueueCreateInfo> &queuesCreateInfo
  ) {
    const size_t queueCount = queueFamilies.size();
    std::vector<std::uint32_t> usedQueueCount(queueCount, 0);

    graphicsQueue = findGraphicsQueue(usedQueueCount);
    computeQueues = findComputeQueue(usedQueueCount, maxComputeQueue);
    transferQueues = findTransferQueue(usedQueueCount, maxTransferQueue);

    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    for (size_t queueIndex = 0; queueIndex < queueCount; queueIndex++) {
      queueCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(queueIndex);
      queueCreateInfo.queueCount = usedQueueCount[queueIndex];
      if (queueCreateInfo.queueCount > 0)
        queuesCreateInfo.push_back(queueCreateInfo);
    }
  }

  VulkanQueue VulkanDevice::findGraphicsQueue(std::vector<std::uint32_t> &usedQueues) const {
    const size_t queueCount = queueFamilies.size();
    for (size_t queueIndex = 0; queueIndex < queueCount; queueIndex++) {
      if (queueFamilies[queueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
          usedQueues[queueIndex] < queueFamilies[queueIndex].queueCount) {
        usedQueues[queueIndex]++;
        return {
          VulkanQueue::Type::Graphics,
          static_cast<std::uint32_t>(queueIndex),
          usedQueues[queueIndex]
        };
      }
    }

    LOG_CORE_WARNING("GPU does not expose Graphics queue. Cannot be used for rendering.");

    return {VulkanQueue::Type::Graphics, 0, 0};
  }

  std::vector<VulkanQueue> VulkanDevice::findComputeQueue(
    std::vector<std::uint32_t> &inOutUsedQueueCount,
    std::uint32_t maxQueueCount
  ) const {
    const size_t numQueues = queueFamilies.size();
    std::vector<VulkanQueue> queues;
    for (size_t queueIndex = 0; queueIndex < numQueues && computeQueues.size() < maxQueueCount; queueIndex++) {
      if (queueFamilies[queueIndex].queueFlags & VK_QUEUE_COMPUTE_BIT &&
      inOutUsedQueueCount[queueIndex] < queueFamilies[queueIndex].queueCount) {
        queues.push_back(
          VulkanQueue(
            VulkanQueue::Type::Compute,
            static_cast<std::uint32_t>(queueIndex),
            inOutUsedQueueCount[queueIndex])
        );

        inOutUsedQueueCount[queueIndex]++;
      }
    }

    return queues;
  }

  std::vector<VulkanQueue> VulkanDevice::findTransferQueue(
    std::vector<std::uint32_t> &inOutUsedQueueCount,
    std::uint32_t maxQueueCount
  ) const {
    const size_t numQueues = queueFamilies.size();
    std::vector<VulkanQueue> queues;
    for( size_t i = 0u; i < numQueues && transferQueues.size() < maxQueueCount; i++) {
      if(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
          !(queueFamilies[i].queueFlags & ( VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT ) ) &&
          inOutUsedQueueCount[i] < queueFamilies[i].queueCount ) {
        queues.push_back(
          VulkanQueue(VulkanQueue::Type::Transfer, static_cast<std::uint32_t>(i),
          inOutUsedQueueCount[i])
        );
        inOutUsedQueueCount[i]++;
      }
    }

    return queues;
  }
}

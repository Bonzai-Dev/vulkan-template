#include <core/application/logger.hpp>
#include "device.hpp"
#include "volk.h"
#include "queue.hpp"
#include "vulkan.hpp"

namespace Core::Graphics {
  VulkanDevice::VulkanDevice(VkInstance &vulkanInstance) : vulkanInstance(vulkanInstance) {
  }

  VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(logicalDevice, nullptr);
  }

  void VulkanDevice::initialize(const std::vector<const char*> &extensions, std::uint32_t deviceId) {
    createPhysicalDevice(deviceId);
    createLogicalDevice();
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

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());
    physicalDevice = devices[deviceId];

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    LOG_CORE_INFO("Rendering using {}.", getName());

    const std::uint32_t version = deviceProperties.apiVersion;
    LOG_CORE_DEBUG(
      "{} supports Vulkan up to {}.{}.{}.",
      getName(),
      VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version),
      VK_VERSION_PATCH(version)
    );

    if (!deviceFeatures.geometryShader)
      supportedStages ^= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

    if (!deviceFeatures.tessellationShader)
      supportedStages ^= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
          VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
  }

  VkResult VulkanDevice::createLogicalDevice() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Creating queues
    std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo;
    // Track the number of queues for each family. The index is the same as the queue family index.
    std::vector<std::uint32_t> usedQueuesCount(queueFamilyCount, 0);

    graphicsQueue = findGraphicsQueue(usedQueuesCount);
    computeQueues = findComputeQueue(usedQueuesCount);
    transferQueues = findTransferQueues(usedQueuesCount);

    for (size_t queueIndex = 0; queueIndex < queueFamilyCount; queueIndex++) {
      VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = static_cast<std::uint32_t>(queueIndex),
        .queueCount = usedQueuesCount[queueIndex]
      };

      if (deviceQueueCreateInfo.queueCount > 0)
        queuesCreateInfo.push_back(deviceQueueCreateInfo);
    }

    std::vector<std::vector<float> > queuePriorities;
    queuePriorities.resize(queuesCreateInfo.size());
    for (size_t infoIndex = 0; infoIndex < queuesCreateInfo.size(); infoIndex++) {
      queuePriorities[infoIndex].resize(queuesCreateInfo[infoIndex].queueCount, 1.0f);
      queuesCreateInfo[infoIndex].pQueuePriorities = queuePriorities[infoIndex].data();
    }

    const std::vector<const char*> deviceExtensions = getExtensions();
    const VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = static_cast<std::uint32_t>(queuesCreateInfo.size()),
      .pQueueCreateInfos = &queuesCreateInfo[0],
      .enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
      .pEnabledFeatures = &deviceFeatures
    };

    const VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to create logical device for {}.", getName());
      return result;
    }

    volkLoadDevice(logicalDevice);

    return result;
  }

  VulkanQueue VulkanDevice::findGraphicsQueue(std::vector<std::uint32_t> &usedQueuesCount) const {
    const size_t familyCount = queueFamilyProperties.size();
    for (size_t familyIndex = 0; familyIndex < familyCount; familyIndex++) {
      if (queueFamilyProperties[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
          usedQueuesCount[familyIndex] < queueFamilyProperties[familyIndex].queueCount) {
        usedQueuesCount[familyIndex]++;

        LOG_CORE_TRACE(
          "Found graphics queue in family {} with {} queues.",
          familyIndex,
          queueFamilyProperties[familyIndex].queueCount
        );

        return {
          VulkanQueue::Type::Graphics,
          static_cast<std::uint32_t>(familyIndex),
          usedQueuesCount[familyIndex]
        };
      }
    }

    LOG_CORE_CRITICAL("GPU does not expose Graphics queue. Cannot be used for rendering.");

    return {VulkanQueue::Type::Graphics, 0, 0};
  }

  std::vector<VulkanQueue> VulkanDevice::findComputeQueue(std::vector<std::uint32_t> &usedQueuesCount) const {
    const size_t familyCount = queueFamilyProperties.size();
    std::vector<VulkanQueue> queues;
    for (size_t familyIndex = 0; familyIndex < familyCount; familyIndex++) {
      if (queueFamilyProperties[familyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT &&
          usedQueuesCount[familyIndex] < queueFamilyProperties[familyIndex].queueCount) {
        LOG_CORE_TRACE("Found compute queue in family {} with {} queues.", familyIndex,
                       queueFamilyProperties[familyIndex].queueCount);

        usedQueuesCount[familyIndex]++;
        queues.emplace_back(
          VulkanQueue::Type::Compute,
          static_cast<std::uint32_t>(familyIndex),
          usedQueuesCount[familyIndex]
        );
      }
    }

    return queues;
  }

  std::vector<VulkanQueue> VulkanDevice::findTransferQueues(std::vector<std::uint32_t> &usedQueueCount) const {
    const size_t familyCount = queueFamilyProperties.size();
    std::vector<VulkanQueue> queues;
    for (size_t familyIndex = 0; familyIndex < familyCount; familyIndex++) {
      if (queueFamilyProperties[familyIndex].queueFlags & VK_QUEUE_TRANSFER_BIT &&
          !(queueFamilyProperties[familyIndex].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) &&
          usedQueueCount[familyIndex] < queueFamilyProperties[familyIndex].queueCount) {
        LOG_CORE_TRACE("Found transfer queue in family {} with {} queues.", familyIndex,
                       queueFamilyProperties[familyIndex].queueCount);

        usedQueueCount[familyIndex]++;
        queues.emplace_back(
          VulkanQueue::Type::Transfer,
          static_cast<std::uint32_t>(familyIndex),
          usedQueueCount[familyIndex]
        );
      }
    }

    return queues;
  }

  void VulkanDevice::initializeQueues() {
    VkQueue queue = nullptr;
    vkGetDeviceQueue(logicalDevice, graphicsQueue.familyIndex, graphicsQueue.queueIndex, &queue);
    graphicsQueue.initialize(logicalDevice, queue);

    for (auto &computeQueue: computeQueues) {
      vkGetDeviceQueue(logicalDevice, computeQueue.familyIndex, computeQueue.queueIndex, &queue);
      computeQueue.initialize(logicalDevice, queue);
    }

    for (auto &transferQueue: transferQueues) {
      vkGetDeviceQueue(logicalDevice, transferQueue.familyIndex, transferQueue.queueIndex, &queue);
      transferQueue.initialize(logicalDevice, queue);
    }

    presentQueue = graphicsQueue.queue;
  }

  std::vector<const char*> VulkanDevice::getExtensions() const {
    static bool foundExtensions = false;
    static std::vector<const char*> extensions;

    if (foundExtensions)
      return extensions;

    std::uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    for (size_t extensionIndex = 0; extensionIndex < extensionCount; extensionIndex++) {
      const std::string extensionName = availableExtensions[extensionIndex].extensionName;
      LOG_CORE_TRACE("Found device extension \"{}\"", extensionName);

      if (extensionName == VK_KHR_MAINTENANCE2_EXTENSION_NAME) {
        extensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
        // bCanRestrictImageViewUsage = true;
      }
      else if (extensionName == VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME)
        extensions.push_back(VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME);
      else if (extensionName == VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) {
        extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        // mRealCapabilities->setCapability(RSC_VP_RT_INDEX_ANY_SHADER);
      }

#ifdef VK_EXT_mesh_shader
      else if (extensionName == VK_EXT_MESH_SHADER_EXTENSION_NAME) {
        extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        // mRealCapabilities->setCapability(RSC_MESH_PROGRAM);
        //
        // mDescriptorSetBindings[0].stageFlags |= VK_SHADER_STAGE_MESH_BIT_NV;
      }
#endif

      if(Vulkan::validationLayersSupported())
        extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

      extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    foundExtensions = true;
    return extensions;
  }
}

#pragma once
#include <vector>
#include "queue.hpp"
#include "volk.h"

namespace Core::Renderer {
  class VulkanDevice {
    public:
      // TODO: ADD CONSTRUCTOR
      VulkanDevice();
      // VulkanDevice() = default;

      VkResult createInstance(
        const std::vector<const char*> &extensions,
        const std::vector<const char*> &layers
      );

      void destroy() const;

      static VkResult createDebugUtilsMessenger(
        const VkInstance &instance,
        const VkDebugUtilsMessengerCreateInfoEXT &createInfo,
        const VkAllocationCallbacks *allocator,
        VkDebugUtilsMessengerEXT *debugMessenger
      );

      static void deleteDebugUtilsMessenger(
        const VkInstance &instance,
        const VkAllocationCallbacks *allocator,
        const VkDebugUtilsMessengerEXT *debugMessenger
      );

      VkInstance getInstance() const { return vulkanInstance; }

      void createPhysicalDevice(std::uint32_t deviceId);

      void createLogicalDevice(
        std::vector<const char*> &extensions,
        std::uint32_t maxComputeQueues,
        std::uint32_t maxTransferQueues
      );

    private:
      VulkanQueue findGraphicsQueue(std::vector<std::uint32_t> &usedQueues) const;

      std::vector<VulkanQueue> findComputeQueue(
        std::vector<std::uint32_t> &inOutUsedQueueCount,
        std::uint32_t maxQueueCount
      ) const;

      std::vector<VulkanQueue> findTransferQueue(
        std::vector<std::uint32_t> &inOutUsedQueueCount,
        std::uint32_t maxQueueCount
      ) const;

      void fillQueueCreationInfo(
        std::uint32_t maxComputeQueue,
        std::uint32_t maxTransferQueue,
        std::vector<VkDeviceQueueCreateInfo> &queuesCreateInfo
      );

      bool validationLayersEnabled = false;
      VkInstance vulkanInstance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
      VkPhysicalDeviceFeatures deviceFeatures{};

      std::uint32_t supportedStages = 0xFFFFFFFF;
      std::vector<VkQueueFamilyProperties> queueFamilies;

      // Graphics queue is *guaranteed by spec* to also be able to run compute and transfer
      // A GPU may not have a graphics queue though
      VulkanQueue graphicsQueue;
      // Additional compute queues to run async compute (besides the main graphics one)
      std::vector<VulkanQueue> computeQueues;
      // Additional transfer queues to run async transfers (besides the main graphics one)
      std::vector<VulkanQueue> transferQueues;

      VkDebugUtilsMessengerCreateInfoEXT createDebugInfo();
  };
}

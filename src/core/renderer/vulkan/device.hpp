#pragma once
#include <vector>
#include <cstdint>
#include "queue.hpp"
#include "volk.h"

namespace Core::Graphics {
  class VulkanDevice {
    public:
      VulkanDevice(VkInstance &vulkanInstance);

      ~VulkanDevice();

      void initialize(const std::vector<const char*> &extensions, std::uint32_t deviceId);

      const char *getName() const { return deviceProperties.deviceName; }

      const VkDevice &getVkDevice() const { return logicalDevice; }

      const VkPhysicalDevice &getPhysicalDevice() const { return physicalDevice; }

      const VulkanQueue &getGraphicsQueue() const { return graphicsQueue; }

      // Gets the device extensions, no the ones from Vulkan
      std::vector<const char*> getExtensions() const;

      void createPhysicalDevice(std::uint32_t deviceId);

      VkResult createLogicalDevice();

      void initializeQueues();

    private:
      VulkanQueue findGraphicsQueue(std::vector<std::uint32_t> &usedQueuesCount) const;

      std::vector<VulkanQueue> findComputeQueue(std::vector<std::uint32_t> &usedQueuesCount) const;

      std::vector<VulkanQueue> findTransferQueues(std::vector<std::uint32_t> &usedQueueCount) const;

      bool validationLayersEnabled = false;
      VkInstance &vulkanInstance;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkPhysicalDeviceProperties deviceProperties{};
      VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
      VkPhysicalDeviceFeatures deviceFeatures{};
      VkDevice logicalDevice = VK_NULL_HANDLE;

      std::uint32_t supportedStages = 0xFFFFFFFF;
      std::vector<VkQueueFamilyProperties> queueFamilyProperties;

      VkQueue presentQueue = VK_NULL_HANDLE;
      // Graphics queue is *guaranteed by spec* to also be able to run compute and transfer
      // A GPU may not have a graphics queue though
      VulkanQueue graphicsQueue = {VulkanQueue::Type::Graphics, 0, 0};
      // Additional compute queues to run async compute (besides the main graphics one)
      std::vector<VulkanQueue> computeQueues;
      // Additional transfer queues to run async transfers (besides the main graphics one)
      std::vector<VulkanQueue> transferQueues;
  };
}

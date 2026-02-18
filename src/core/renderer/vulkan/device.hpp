#pragma once
#include <vector>
#include "volk.h"

namespace Core::Renderer {
  class VulkanDevice {
    public:
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

      void createLogicalDevice();

      void findGraphicsQueue();

    private:
      bool validationLayersEnabled = false;
      VkInstance vulkanInstance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
      VkPhysicalDeviceFeatures deviceFeatures{};

      std::uint32_t supportedStages = 0xFFFFFFFF;
      std::vector<VkQueueFamilyProperties> queueFamilies;

      VkDebugUtilsMessengerCreateInfoEXT createDebugInfo();
  };
}

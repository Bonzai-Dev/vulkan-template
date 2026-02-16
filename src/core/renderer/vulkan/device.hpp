#pragma once
#include <vector>
#include "volk.h"

namespace Core::Renderer {
  class VulkanDevice {
    public:
      void createInstance(
        const std::vector<const char*> &extensions,
        const std::vector<const char*> &layers,
        PFN_vkDebugUtilsMessengerCallbackEXT debugCallback
      );

      void destroyInstance() const;

      VkInstance getInstance() const { return vulkanInstance; }

    private:
      bool validationLayersEnabled = false;
      VkInstance vulkanInstance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

      VkResult deleteDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
      );

      VkResult createDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
      );

      void createDebugCallback();
  };
}

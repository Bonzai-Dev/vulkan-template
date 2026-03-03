#pragma once
#include <vulkan/vulkan.hpp>
#include "device.hpp"

namespace Core::Graphics {
  class Vulkan {
    public:
      Vulkan();

      ~Vulkan();

#ifdef NDEBUG
      static constexpr bool debugEnabled = false;
#else
      static constexpr bool debugEnabled = true;
#endif

      static std::vector<const char*> getExtensions();

      const VulkanDevice &getDevice() { return vulkanDevice; }

      const VkDevice &getVkDevice() const { return vulkanDevice.getVkDevice(); }

      const VkPhysicalDevice &getPhysicalDevice() const { return vulkanDevice.getPhysicalDevice(); }

      const VkInstance &getVkInstance() const { return vulkanInstance; }

      static bool validationLayersSupported() { return validationLayersEnabled; }

    private:
      static VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData
      );

      VkResult createInstance(
        const std::vector<const char*> &extensions,
        const std::vector<const char*> &layers
      );

      static inline bool validationLayersEnabled = false;

      VkInstance vulkanInstance = VK_NULL_HANDLE;
      VulkanDevice vulkanDevice = {vulkanInstance};
      VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
      std::vector<const char*> instanceLayers;
  };
}

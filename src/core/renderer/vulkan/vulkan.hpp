#pragma once
#include <vulkan/vulkan.hpp>
#include "device.hpp"

namespace Core::Renderer {
  class Vulkan {
    public:
      ~Vulkan();

      static void initialize();

      static bool validationLayerSupported();

      static std::vector<const char*> getExtensions();

      static const VulkanDevice &getInstance() { return vulkanDevice; }

      static void createInstance();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
      );

      static VkResult createDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
      );

      static void destroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
      );

    private:
      static inline VulkanDevice vulkanDevice;
      #ifdef NDEBUG
        static constexpr bool validationLayersEnabled = false;
      #else
        static constexpr bool validationLayersEnabled = true;
      #endif
  };
}

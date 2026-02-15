#pragma once
#include <vulkan/vulkan.hpp>
#include "device.hpp"

namespace Core::Renderer {
  class Vulkan {
    public:
      ~Vulkan();

      #ifdef NDEBUG
        static constexpr bool debugEnabled = false;
      #else
        static constexpr bool debugEnabled = true;
      #endif

      static void initialize();

      static std::vector<const char*> getExtensions();

      static const VulkanDevice &getDevice() { return vulkanDevice; }

      static bool validationLayersSupported() { return validationLayersEnabled; }

      static void createInstance();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
      );
      //
      // static VkResult createDebugUtilsMessenger(
      //   VkInstance instance,
      //   const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      //   const VkAllocationCallbacks *pAllocator,
      //   VkDebugUtilsMessengerEXT *pDebugMessenger
      // );
      //
      // static void destroyDebugUtilsMessenger(
      //   VkInstance instance,
      //   VkDebugUtilsMessengerEXT debugMessenger,
      //   const VkAllocationCallbacks *pAllocator
      // );
      //
      // static void createDebugCallback();

    private:
      static inline VulkanDevice vulkanDevice;
      static inline std::vector<const char *> instanceLayers;
      static inline bool validationLayersEnabled = false;
  };
}

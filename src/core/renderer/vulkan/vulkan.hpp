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

      static void setupDebugMessenger();

      static std::vector<const char*> getExtensions();

      static const VulkanDevice &getDevice() { return vulkanDevice; }

      static bool validationLayersSupported() { return validationLayersEnabled; }

      static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkDebugUtilsMessengerEXT *pDebugMessenger);

      static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                         const VkAllocationCallbacks *pAllocator);

      static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

      static bool checkValidationLayerSupport();

      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
      );

    private:
      static inline VulkanDevice vulkanDevice;
      static inline std::vector<const char *> instanceLayers;
      static inline bool validationLayersEnabled = false;

      static inline VkInstance vulkanInstance = VK_NULL_HANDLE;
      static inline VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
  };
}

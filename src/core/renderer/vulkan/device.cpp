#include <core/application/logger.hpp>
#include "device.hpp"
#include "volk.h"
#include "vulkan.hpp"

namespace Core::Renderer {
  VkResult VulkanDevice::createInstance(
    const std::vector<const char*> &extensions,
    const std::vector<const char*> &layers
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
}

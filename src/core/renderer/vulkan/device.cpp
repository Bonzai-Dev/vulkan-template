#include <core/application/logger.hpp>
#include "device.hpp"
#include "volk.h"
#include "vulkan.hpp"

namespace Core::Renderer {
  void VulkanDevice::createInstance(
    const std::vector<const char*> &extensions,
    const std::vector<const char*> &layers,
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback
  ) {
    validationLayersEnabled = Vulkan::validationLayersSupported() && Vulkan::debugEnabled;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (validationLayersEnabled) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
      createInfo.ppEnabledLayerNames = layers.data();

      debugCreateInfo = {
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
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
      LOG_CORE_TRACE("Vulkan validation layers requested, but not available.");
    }

    if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS)
      throw std::runtime_error("failed to create instance!");

    volkLoadInstance(vulkanInstance);

    if (validationLayersEnabled) {
      VkDebugUtilsMessengerCreateInfoEXT createInfo;
      createInfo = {
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

      auto vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugUtilsMessengerEXT")
      );

      if (vkCreateDebugUtilsMessenger != nullptr) {
        if (vkCreateDebugUtilsMessenger(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
          LOG_CORE_ERROR("Failed to setup debugger.");
        }

        LOG_CORE_INFO("Vulkan debugger setup successfully.");
      } else {
        throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT function!");
      }
    }
  }

  void VulkanDevice::destroyInstance() const {
    if (validationLayersEnabled) {
      static const auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugUtilsMessengerEXT")
      );
      if (destroyMessenger != nullptr)
        destroyMessenger(vulkanInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(vulkanInstance, nullptr);
  }

  VkResult VulkanDevice::createDebugUtilsMessenger(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
  ) {
    static const auto vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugUtilsMessengerEXT")
    );

    if (vkCreateDebugUtilsMessenger != nullptr) {
      if (vkCreateDebugUtilsMessenger(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        LOG_CORE_ERROR("Failed to attach debugger.");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
      }

      LOG_CORE_INFO("Vulkan debugger attach successfully.");
      return VK_SUCCESS;
    }

    LOG_CORE_ERROR("Failed to attach debugger. \"vkCreateDebugUtilsMessengerEXT\" extension not present.");
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  VkResult VulkanDevice::deleteDebugUtilsMessenger(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
  ) {
    static const auto vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
     vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugUtilsMessengerEXT")
   );
    if (vkDestroyDebugUtilsMessenger != nullptr) {
      if (vkDestroyDebugUtilsMessenger(vulkanInstance, debugMessenger, nullptr) != VK_SUCCESS) {
        LOG_CORE_ERROR("Failed to delete debugger.");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
      }
    }

    LOG_CORE_ERROR("Failed to attach debugger. \"vkDestroyDebugUtilsMessengerEXT\" extension not present.");
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

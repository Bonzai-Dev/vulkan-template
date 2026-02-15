#include <vulkan/vulkan.hpp>
#include <core/application/logger.hpp>
#include "device.hpp"

#include <iostream>

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

      createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
      createInfo.ppEnabledLayerNames = layers.data();
      createInfo.pNext = &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
      LOG_CORE_TRACE("Vulkan validation layers requested, but not available.");
    }

    if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS)
      throw std::runtime_error("failed to create instance!");

    static const auto vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(vulkanInstance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (vkCreateDebugUtilsMessenger)
      vkCreateDebugUtilsMessenger(vulkanInstance, &debugCreateInfo, nullptr, &debugMessenger);

    uint32_t physCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &physCount, nullptr);
    if (physCount == 0) {
      std::cout << "No physical devices found" << std::endl;
      return;
    }
    std::vector<VkPhysicalDevice> physDevices(physCount);
    vkEnumeratePhysicalDevices(vulkanInstance, &physCount, physDevices.data());

    VkPhysicalDevice phys = physDevices[0];
    VkDeviceCreateInfo badDeviceInfo{};
    badDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    badDeviceInfo.queueCreateInfoCount = 1;
    badDeviceInfo.pQueueCreateInfos = nullptr;

    VkDevice badDevice;
    vkCreateDevice(phys, &badDeviceInfo, nullptr, &badDevice);
  }

  void VulkanDevice::destroy() const {
    if (validationLayersEnabled) {
      static const auto vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(vulkanInstance, "vkDestroyDebugUtilsMessengerEXT")
      );
      if (vkDestroyDebugUtilsMessenger != nullptr)
        vkDestroyDebugUtilsMessenger(vulkanInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(vulkanInstance, nullptr);
  }
}

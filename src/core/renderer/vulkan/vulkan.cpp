#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_hints.h>
#include <core/application/logger.hpp>
#include "volk.h"
#include "vulkan.hpp"

namespace Core::Graphics {
  Vulkan::Vulkan() {
    if (vulkanInstance)
      return;

    if (volkInitialize() != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to load Vulkan. Vulkan drivers may be missing on your system.");
      return;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
      LOG_CORE_CRITICAL("Failed to initialize SDL Video: {}", SDL_GetError());
      return;
    }

    // Adding vulkan layers
    uint32_t instanceLayerPropertyCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());
    for (size_t layerIndex = 0; layerIndex < instanceLayerPropertyCount; layerIndex++) {
      const std::string layerName = instanceLayerProperties[layerIndex].layerName;
      LOG_CORE_DEBUG("Found instance layer: {}", layerName);
      if (debugEnabled && layerName == "VK_LAYER_KHRONOS_validation") {
        validationLayersEnabled = true;
        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        LOG_CORE_DEBUG("Enabled Vulkan validation layer.");
      }
    }

    if (createInstance(getExtensions(), instanceLayers) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to create Vulkan instance.");
      return;
    }

    vulkanDevice.initialize(getExtensions(), 0);

    uint32_t version = 0;
    vkEnumerateInstanceVersion(&version);
    LOG_CORE_INFO(
      "Running Vulkan at version {}.{}.{}.",
      VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version),
      VK_VERSION_PATCH(version)
    );
  }

  Vulkan::~Vulkan() {
    if (validationLayersEnabled)
      vkDestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
  }

  VkResult Vulkan::createInstance(
    const std::vector<const char*> &extensions,
    const std::vector<const char*> &layers
  ) {
    validationLayersEnabled = validationLayersSupported() && debugEnabled;

    constexpr VkApplicationInfo appInfo {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Vulkan Application",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Vulkan engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 3, 0),
    };

    VkInstanceCreateInfo createInfo {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()
    };

    // Setting up debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    if (validationLayersEnabled) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
      createInfo.ppEnabledLayerNames = layers.data();

      debugInfo = {
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
      createInfo.pNext = &debugInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
      LOG_CORE_TRACE("Vulkan validation layers requested, but not available.");
    }

    const VkResult instanceResult = vkCreateInstance(&createInfo, nullptr, &vulkanInstance);
    if (instanceResult != VK_SUCCESS) {
      LOG_CORE_ERROR("Failed to create Vulkan instance.");
      return instanceResult;
    }

    volkLoadInstanceOnly(vulkanInstance);

    if (validationLayersEnabled)
      vkCreateDebugUtilsMessengerEXT(vulkanInstance, &debugInfo, nullptr, &debugMessenger);

    return instanceResult;
  }

  VkBool32 VKAPI_ATTR Vulkan::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData
  ) {
    switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_CORE_TRACE("Vulkan diagnostic: {}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_CORE_INFO("Vulkan info: {}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_CORE_WARNING("Vulkan warning: {}", callbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_CORE_ERROR("Vulkan error: {}", callbackData->pMessage);
        break;

      default:
        break;
    }

    return VK_FALSE;
  }

  std::vector<const char*> Vulkan::getExtensions() {
    static bool foundExtensions = false;
    static uint32_t extensionCount = 0;
    static char const *const*extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    static std::vector<const char*> extensionList(extensions, extensions + extensionCount);

    if (foundExtensions)
      return extensionList;

    if (validationLayersEnabled) {
      extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      extensionList.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    foundExtensions = true;
    return extensionList;
  }
}

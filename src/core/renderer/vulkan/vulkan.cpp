#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "vulkan.hpp"

namespace Core::Renderer {
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  VkResult Vulkan::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void Vulkan::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                             const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
    }
  }

  Vulkan::~Vulkan() {
    if (validationLayersEnabled) {
      DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
    }
    vkDestroyInstance(vulkanInstance, nullptr);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
  }

  void Vulkan::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      LOG_CORE_CRITICAL("Failed to initialize SDL Video: {}", SDL_GetError());
      return;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
      LOG_CORE_CRITICAL("Failed to load Vulkan: {}", SDL_GetError());
      return;
    }

    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }
    validationLayersEnabled = true;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
  }

  void Vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  void Vulkan::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger!");
    }
  }

  std::vector<const char*> Vulkan::getExtensions() {
    uint32_t extensionCount = 0;
    char const *const*extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    std::vector<const char*> extensionList(extensions, extensions + extensionCount);

    if (validationLayersEnabled)
      extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensionList;
  }

  bool Vulkan::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: validationLayers) {
      bool layerFound = false;

      for (const auto &layerProperties: availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  VKAPI_ATTR VkBool32 Vulkan::debugCallback(
     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
     VkDebugUtilsMessageTypeFlagsEXT messageType,
     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
     void *pUserData
   ) {
    switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_CORE_TRACE("Vulkan diagnostic: {}", pCallbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_CORE_INFO("Vulkan info: {}", pCallbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_CORE_WARNING("Vulkan warning: {}", pCallbackData->pMessage);
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_CORE_ERROR("Vulkan error: {}", pCallbackData->pMessage);
        break;

      default:
        break;
    }

    return VK_FALSE;
  }
}

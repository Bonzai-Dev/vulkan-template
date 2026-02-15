#include <iostream>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_hints.h>
#include <core/application/logger.hpp>
#include "vulkan.hpp"

namespace Core::Renderer {
  void Vulkan::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      LOG_CORE_CRITICAL("Failed to initialize SDL Video: {}", SDL_GetError());
      return;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
      LOG_CORE_CRITICAL("Failed to load Vulkan: {}", SDL_GetError());
      return;
    }

    SDL_SetHint(SDL_HINT_RENDER_VULKAN_DEBUG, "1");
    createInstance();

    LOG_CORE_INFO("Successfully initialized Vulkan.");
  }

  Vulkan::~Vulkan() {
    vulkanDevice.destroyInstance();
  }

  void Vulkan::createInstance() {
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
      }
    }

    vulkanDevice.createInstance(getExtensions(), instanceLayers, debugCallback);
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

  // VkResult Vulkan::createDebugUtilsMessenger(
  //   VkInstance instance,
  //   const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
  //   const VkAllocationCallbacks *pAllocator,
  //   VkDebugUtilsMessengerEXT *pDebugMessenger
  // ) {
  //   const static auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
  //     vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
  //   );
  //   if (createMessenger != nullptr) {
  //     return createMessenger(instance, pCreateInfo, pAllocator, pDebugMessenger);
  //   } else {
  //     return VK_ERROR_EXTENSION_NOT_PRESENT;
  //   }
  // }


  // void Vulkan::destroyDebugUtilsMessenger(
  //   VkInstance instance,
  //   VkDebugUtilsMessengerEXT debugMessenger,
  //   const VkAllocationCallbacks *pAllocator
  // ) {
  //   static const auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
  //     vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
  //   );
  //   if (destroyMessenger != nullptr)
  //     destroyMessenger(instance, debugMessenger, pAllocator);
  // }

  std::vector<const char*> Vulkan::getExtensions() {
    uint32_t extensionCount = 0;
    char const *const*extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    std::vector<const char*> extensionList(extensions, extensions + extensionCount);

    if (validationLayersEnabled)
      extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensionList;
  }
}

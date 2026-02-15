#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
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

    createInstance();

    LOG_CORE_INFO("Successfully initialized Vulkan.");
  }

  Vulkan::~Vulkan() {
    vulkanDevice.destroyInstance();
  }

  void Vulkan::createInstance() {
    if (validationLayersEnabled && !validationLayerSupported())
      LOG_CORE_TRACE("Validation layers requested, but not available.");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    const std::vector<const char*> extensions = getExtensions();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (validationLayersEnabled && validationLayerSupported()) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
      createInfo.ppEnabledLayerNames = ValidationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }
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

  VkResult Vulkan::createDebugUtilsMessenger(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
  ) {
    const static auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (createMessenger != nullptr)
      return createMessenger(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
      return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  void Vulkan::destroyDebugUtilsMessenger(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator
  ) {
    static const auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (destroyMessenger != nullptr)
      destroyMessenger(instance, debugMessenger, pAllocator);
  }

  bool Vulkan::validationLayerSupported() {
    static bool called = false;
    static bool layerFound = false;
    if (called)
      return layerFound;

    static uint32_t instanceLayerPropertyCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    static std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());

    for (const auto &layerName: ValidationLayers) {
      for (const auto &layerProperties: instanceLayerProperties) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          called = true;
          return true;
        }
      }

      if (!layerFound) {
        LOG_CORE_DEBUG("Vulkan validation layer not found: \"{}\".", layerName);
        called = true;
        return false;
      }
    }

    return true;
  }

  std::vector<const char*> Vulkan::getExtensions() {
    static uint32_t extensionCount = 0;
    static char const *const*extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    static std::vector<const char*> extensionList(extensions, extensions + extensionCount);

    if (validationLayersEnabled)
      extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensionList;
  }
}

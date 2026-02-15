// #include <vulkan/vulkan.hpp>
// #include "../../application/logger.hpp"
// #include "debugger.hpp"
// #include "vulkan.hpp"
//
// namespace Core::Renderer {
//   VulkanDebugger::VulkanDebugger() {
//     VkDebugUtilsMessengerCreateInfoEXT createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//     createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
//                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
//                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
//     createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
//                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
//                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//     createInfo.pfnUserCallback = debugCallback;
//     createInfo.pUserData = nullptr;
//     createInfo.pNext = &createInfo;
//
//     if (createDebugUtilsMessenger(Vulkan::getInstance(), &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
//       LOG_CORE_TRACE("Failed to set up Vulkan debug messenger.");
//   }
//
//   VulkanDebugger::~VulkanDebugger() {
//     destroyDebugUtilsMessenger(Vulkan::getInstance(), debugMessenger, nullptr);
//   }
//
//   VkResult VulkanDebugger::createDebugUtilsMessenger(
//     VkInstance instance,
//     const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
//     const VkAllocationCallbacks *pAllocator,
//     VkDebugUtilsMessengerEXT *pDebugMessenger
//   ) {
//     const static auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
//       vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
//     );
//     if (createMessenger != nullptr)
//       return createMessenger(instance, pCreateInfo, pAllocator, pDebugMessenger);
//     else
//       return VK_ERROR_EXTENSION_NOT_PRESENT;
//   }
//
//   void VulkanDebugger::destroyDebugUtilsMessenger(
//     VkInstance instance,
//     VkDebugUtilsMessengerEXT debugMessenger,
//     const VkAllocationCallbacks *pAllocator
//   ) {
//     static const auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
//       vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
//     );
//     if (destroyMessenger != nullptr)
//       destroyMessenger(instance, debugMessenger, pAllocator);
//   }
//
//   VKAPI_ATTR VkBool32 VulkanDebugger::debugCallback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//     VkDebugUtilsMessageTypeFlagsEXT messageType,
//     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
//     void *pUserData
//   ) {
//     switch (messageSeverity) {
//       case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
//         LOG_CORE_TRACE("Vulkan diagnostic: {}", pCallbackData->pMessage);
//         break;
//
//       case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
//         LOG_CORE_INFO("Vulkan info: {}", pCallbackData->pMessage);
//         break;
//
//       case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
//         LOG_CORE_WARNING("Vulkan warning: {}", pCallbackData->pMessage);
//         break;
//
//       case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
//         LOG_CORE_ERROR("Vulkan error: {}", pCallbackData->pMessage);
//         break;
//
//       default:
//         break;
//     }
//
//     return VK_FALSE;
//   }
// }

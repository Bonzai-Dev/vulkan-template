#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "vulkan.hpp"
#include "volk.h"
#include "window.hpp"

namespace Core::Graphics {
  VulkanWindow::VulkanWindow(VkInstance &vulkanInstance, VulkanDevice &device) :
  vulkanDevice(device), vulkanInstance(vulkanInstance) {

  }

  VulkanWindow::~VulkanWindow() {
    destroy();
  }

  void VulkanWindow::destroy() const {
    vkDestroySurfaceKHR(vulkanInstance, windowSurface, nullptr);
  }

  void VulkanWindow::createSurface() {
    if (!SDL_Vulkan_CreateSurface(SDL_GL_GetCurrentWindow(), vulkanInstance, nullptr, &windowSurface)) {
      LOG_CORE_CRITICAL("Failed to create Vulkan surface: {}", SDL_GetError());
      return;
    }
  }

  void VulkanWindow::createSwapChain() {
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
      vulkanDevice.getPhysicalDevice(),
      vulkanDevice.getGraphicsQueue().familyIndex,
      windowSurface,
      &supported
    );

    if (!supported)
      LOG_CORE_CRITICAL("KHR Surface is unsupported.");
  }
}

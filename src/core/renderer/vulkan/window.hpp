#pragma once
#include "volk.h"

namespace Core::Graphics {
  class VulkanWindow {
    public:
      VulkanWindow(VkInstance &vulkanInstance, VulkanDevice &device);

      ~VulkanWindow();

      void destroy() const;

      void createSurface();

      void createSwapChain();

    private:
      VulkanDevice &vulkanDevice;
      VkInstance &vulkanInstance;
      VkSurfaceKHR windowSurface = VK_NULL_HANDLE;
  };
}
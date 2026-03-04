#pragma once
#include "volk.h"

namespace Core::Graphics {
  class VulkanWindow {
    public:
      VulkanWindow(VkInstance &vulkanInstance, VulkanDevice &device);

      ~VulkanWindow();

      VkSurfaceFormatKHR chooseSurfaceFormat();

      void createSurface();

      void createSwapChain();

      bool vsyncEnabled = true;

    private:
      VkImageView createImageView(
        const VkImage &image,
        const VkFormat &format,
        VkImageAspectFlags aspectFlags,
        VkImageViewType viewType,
        std::uint32_t layerCount,
        std::uint32_t mipLevels
      ) const;

      void destroy();

      void destroySwapChain() const;

      VulkanDevice &vulkanDevice;
      VkInstance &vulkanInstance;

      VkSwapchainKHR swapChain = VK_NULL_HANDLE;
      VkSurfaceKHR windowSurface = VK_NULL_HANDLE;

      std::vector<VkImage> swapChainImages;
      std::vector<VkImageView> swapChainImageViews;
  };
}

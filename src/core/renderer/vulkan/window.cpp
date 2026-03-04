#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "vulkan.hpp"
#include "volk.h"
#include "window.hpp"

namespace Core::Graphics {
  VulkanWindow::VulkanWindow(VkInstance &vulkanInstance, VulkanDevice &device) : vulkanDevice(device),
    vulkanInstance(vulkanInstance) {
  }

  VulkanWindow::~VulkanWindow() {
    destroy();
  }

  void VulkanWindow::destroy() {
    destroySwapChain();

    if (windowSurface) {
      vkDestroySurfaceKHR(vulkanInstance, windowSurface, nullptr);
      windowSurface = nullptr;
    }
  }

  void VulkanWindow::destroySwapChain() const {
    for (size_t imageViewIndex = 0; imageViewIndex < swapChainImageViews.size(); imageViewIndex++)
      vkDestroyImageView(vulkanDevice.getVkDevice(), swapChainImageViews[imageViewIndex], nullptr);

    vkDestroySwapchainKHR(vulkanDevice.getVkDevice(), swapChain, nullptr);
  }

  void VulkanWindow::createSurface() {
    if (!SDL_Vulkan_CreateSurface(SDL_GL_GetCurrentWindow(), vulkanInstance, nullptr, &windowSurface)) {
      LOG_CORE_CRITICAL("Failed to create Vulkan surface: {}", SDL_GetError());
      return;
    }
  }

  VkSurfaceFormatKHR VulkanWindow::chooseSurfaceFormat() {
    std::uint32_t formatsCount = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(
          vulkanDevice.getPhysicalDevice(), windowSurface, &formatsCount, nullptr) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to get surface formats count.");
      return {};
    }

    if (formatsCount == 0) {
      LOG_CORE_CRITICAL("No surface formats found.");
      return {};
    }

    std::vector<VkSurfaceFormatKHR> formats(formatsCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(
          vulkanDevice.getPhysicalDevice(), windowSurface, &formatsCount, formats.data()) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to get surface formats.");
      return {};
    }

    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (size_t formatIndex = 0; formatIndex < formatsCount; formatIndex++) {
      const VkSurfaceFormatKHR &format = formats[formatIndex];
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return formats[formatIndex];
    }

    return chosenFormat;
  }

  void VulkanWindow::createSwapChain() {
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
      vulkanDevice.getPhysicalDevice(),
      vulkanDevice.getGraphicsQueue().familyIndex,
      windowSurface,
      &supported
    );

    if (!supported) {
      LOG_CORE_CRITICAL("KHR Surface is unsupported.");
      return;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          vulkanDevice.getPhysicalDevice(), windowSurface, &surfaceCapabilities) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to get surface capabilities.");
      return;
    }

    // Tries to get triple buffering by default
    // https://github.com/KhronosGroup/Vulkan-Samples/tree/main/samples/performance/swapchain_images
    auto minImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount != 0)
      minImageCount = std::min(minImageCount, surfaceCapabilities.maxImageCount);

    LOG_CORE_INFO("Buffer count: {}", minImageCount);

    std::uint32_t presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      vulkanDevice.getPhysicalDevice(), windowSurface, &presentModesCount, nullptr
    );

    std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      vulkanDevice.getPhysicalDevice(), windowSurface, &presentModesCount, presentModes.data()
    );

    // FIFO is guaranteed to be present
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!vsyncEnabled &&
        std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end()) {
      presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat();
    const VkSwapchainCreateInfoKHR swapChainCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = windowSurface,
      .minImageCount = minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
      .imageExtent = surfaceCapabilities.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
    };

    if (vkCreateSwapchainKHR(vulkanDevice.getVkDevice(), &swapChainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to create swap chain.");
      return;
    }

    std::uint32_t swapChainImageCount = 0;
    if (vkGetSwapchainImagesKHR(vulkanDevice.getVkDevice(), swapChain, &swapChainImageCount, nullptr) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to get swap chain images count.");
      return;
    }

    if (swapChainImageCount == 0) {
      LOG_CORE_WARNING("No swap chain images found.");
      return;
    }

    swapChainImages.resize(swapChainImageCount);
    swapChainImageViews.resize(swapChainImageCount);

    if (vkGetSwapchainImagesKHR(
          vulkanDevice.getVkDevice(), swapChain, &swapChainImageCount, swapChainImages.data()) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to get swap chain images.");
      return;
    }

    for (size_t imageViewIndex = 0; imageViewIndex < swapChainImageCount; imageViewIndex++) {
      swapChainImageViews[imageViewIndex] = createImageView(
        swapChainImages[imageViewIndex],
        surfaceFormat.format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_2D,
        1,
        1
      );
    }
  }

  VkImageView VulkanWindow::createImageView(
    const VkImage &image,
    const VkFormat &format,
    VkImageAspectFlags aspectFlags,
    VkImageViewType viewType,
    std::uint32_t layerCount,
    std::uint32_t mipLevels
  ) const {
    VkImageViewCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = viewType,
      .format = format,
      .components = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .a = VK_COMPONENT_SWIZZLE_IDENTITY
      },
      .subresourceRange = {
        .aspectMask = aspectFlags,
        .baseMipLevel = 0,
        .levelCount = mipLevels,
        .baseArrayLayer = 0,
        .layerCount = layerCount
      }
    };

    VkImageView imageView;
    if (vkCreateImageView(vulkanDevice.getVkDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
      LOG_CORE_CRITICAL("Failed to create image view.");
      return VK_NULL_HANDLE;
    }

    return imageView;
  }
}

#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Core::Renderer {
  class VulkanDevice {
    public:
      void createInstance(
        const std::vector<const char*> &extensions,
        const std::vector<const char*> &layers,
        PFN_vkDebugReportCallbackEXT debugCallback
      );

      void destroyInstance() const;

      VkInstance getInstance() const { return vulkanInstance; }

    private:
      VkInstance vulkanInstance{};
  };
}

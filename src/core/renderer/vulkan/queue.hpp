#pragma once
#include "volk.h"

namespace Core::Graphics {
  class VulkanQueue {
    public:
      enum class Type {
        Graphics,
        Compute,
        Transfer,
        NumQueueFamilies
      };

      VulkanQueue(Type family, std::uint32_t familyIndex, std::uint32_t queueIndex);

      ~VulkanQueue() = default;

      void initialize(VkDevice &device, VkQueue &queue);

      Type queueFamily;

      std::uint32_t familyIndex = 0;
      std::uint32_t queueIndex = 0;

      VkQueue queue = VK_NULL_HANDLE;
      VkDevice device = VK_NULL_HANDLE;
  };
}

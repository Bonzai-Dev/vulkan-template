#include <cstdint>
#include "volk.h"
#include "queue.hpp"

namespace Core::Graphics {
  VulkanQueue::VulkanQueue(Type family, std::uint32_t familyIndex, std::uint32_t queueIndex) :
  queueFamily(family), familyIndex(familyIndex), queueIndex(queueIndex) {
  }

  void VulkanQueue::initialize(VkDevice &device, VkQueue &queue) {
    this->queue = queue;
    this->device = device;
  }
}

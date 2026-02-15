#include "renderer.hpp"
#include "vulkan/vulkan.hpp"

namespace Core {
  void Renderer::initialize(Backend backend) {
    if (backend == Backend::Vulkan)
      Vulkan::initialize();
  }
}

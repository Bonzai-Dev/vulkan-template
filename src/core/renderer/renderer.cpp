#include "renderer.hpp"
#include "vulkan/vulkan.hpp"

namespace Core::Graphics {
  Renderer::Renderer(Backend backend) : backend(backend) {
    if (backend == Backend::Vulkan)
      Vulkan vulkan = Vulkan();
  }
}

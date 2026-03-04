#include <core/application/logger.hpp>
#include "renderer.hpp"
#include "vulkan/vulkan.hpp"

namespace Core::Graphics {
  Renderer::Renderer(Backend backend) {
    this->backend = backend;

    if (backend == Backend::Vulkan) {
      LOG_CORE_INFO("Initializing renderer with Vulkan.");
      Vulkan vulkan = Vulkan();
    }
  }
}

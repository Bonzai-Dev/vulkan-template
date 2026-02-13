#pragma once
#include <vulkan/vulkan.hpp>

namespace Core {
  class Renderer {
    public:
      Renderer() = default;
      ~Renderer() = default;

      static void initialize();
  };
}
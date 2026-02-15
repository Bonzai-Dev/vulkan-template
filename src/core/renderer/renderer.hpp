#pragma once

namespace Core::Renderer {
  enum class Backend {
    Vulkan,
  };

  void initialize(Backend backend);
}

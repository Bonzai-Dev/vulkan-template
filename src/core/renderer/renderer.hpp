#pragma once

namespace Core::Graphics {
  class Renderer {
    public:
      enum class Backend {
        Vulkan,
      };

      Renderer(Backend backend);

      static Backend getBackend() { return backend; }

    private:
      static inline Backend backend;
  };
}

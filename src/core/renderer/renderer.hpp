#pragma once

namespace Core::Graphics {
  class Renderer {
    public:
      enum class Backend {
        Vulkan,
      };

      Renderer(Backend backend);

      Backend getBackend() const { return backend; }

    private:
      Backend backend;
  };
}

#pragma once
#include <core/application/render_layer.hpp>

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void render() override;
};

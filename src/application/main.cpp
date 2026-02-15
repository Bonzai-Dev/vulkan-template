#include <core/application/application.hpp>
#include "app_layer.hpp"

int main() {
  const Core::Application application("Vulkan Template");
  application.addLayer<AppLayer>();

  return 0;
}

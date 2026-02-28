#pragma once

namespace Core::Renderer {
  struct VulkanQueue {
    enum class Type {
      Graphics,
      Compute,
      Transfer,
      NumQueueFamilies
    };

    Type queueFamily;
    std::uint32_t familyIndex = 0;
    std::uint32_t queueIndex = 0;
  };

  // class VulkanQueue {
  //   public:
  //     enum class Type {
  //       Graphics,
  //       Compute,
  //       Transfer,
  //       NumQueueFamilies
  //     };
  //
  //     VulkanQueue(Type family, std::uint32_t familyIndex, std::uint32_t queueIndex);
  //
  //     ~VulkanQueue() = default;
  //
  //     std::uint32_t getFamilyIndex() const { return familyIndex; }
  //
  //     std::uint32_t getQueueIndex() const { return queueIndex; }
  //
  //     Type getQueueFamily() const { return queueFamily; }
  //
  //   private:
  //     Type queueFamily;
  //
  //     std::uint32_t familyIndex = 0;
  //     std::uint32_t queueIndex = 0;
  // };
}
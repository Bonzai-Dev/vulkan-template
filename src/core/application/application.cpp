#include <core/logger.hpp>
#include <core/renderer.hpp>
#include <glm/glm.hpp>
#include "application.hpp"

namespace Core {
  Application::Application(const char *name) {
    Logger::initialize();
    Renderer::initialize();

    mainWindow = std::make_shared<Window>(name, true);
    running = true;
    run();
  }

  void Application::run() const {
    while (running) {
      pollInputs();

      for (const auto &layer: layers)
        layer->render();

      // window->updateBuffer();

      const double currentTime = static_cast<double>(SDL_GetTicks()) / 1000;
      deltaTime = currentTime - lastFrameTime;
      lastFrameTime = static_cast<double>(currentTime);
    }

    quit();
  }

  void Application::pollInputs() const {
    SDL_Event windowEvent;
    mouseDelta = glm::zero<glm::vec2>();
    isMouseMoving = false;

    while (SDL_PollEvent(&windowEvent)) {
      const auto pressedKey = static_cast<Inputs::KeyboardKey>(windowEvent.key.scancode);
      switch (windowEvent.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;

        case SDL_EVENT_KEY_DOWN:
          if (!pressedKeys.contains(pressedKey))
            pressedKeys[pressedKey] = windowEvent.key;
          break;

        case SDL_EVENT_KEY_UP:
          if (pressedKeys.contains(pressedKey))
            pressedKeys.erase(pressedKey);
          break;

        case SDL_EVENT_MOUSE_MOTION:
          isMouseMoving = true;
          mouseDelta = glm::vec2(windowEvent.motion.xrel, windowEvent.motion.yrel);
          break;
      }
    }
  }

  bool Application::keyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const {
    if (!pressedKeys.contains(key)) return false;

    const SDL_KeyboardEvent keyEvent = pressedKeys[key];
    const SDL_Scancode scancode = keyEvent.scancode;
    const SDL_Keycode keycode = SDL_GetKeyFromScancode(keyEvent.scancode, keyEvent.mod, true);

    const bool isScancode = detectMode == Inputs::KeyDetectMode::Keycode && scancode == keyEvent.scancode;
    const bool isKeycode = detectMode == Inputs::KeyDetectMode::Scancode && keycode == keyEvent.key;
    if (isScancode || isKeycode)
      return true;

    return false;
  }

  void Application::quit() const {
    SDL_Quit();
  }
}

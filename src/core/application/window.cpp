#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <core/application/logger.hpp>
#include "window.hpp"

namespace Core {
  Window::Window(const char *windowName, bool mouseLocked) {
    // Get display information
    int displayCount;
    displays = SDL_GetDisplays(&displayCount);
    if (displayCount > 0)
      LOG_CORE_INFO("Found {} display(s).", displayCount);
    else
      LOG_CORE_WARNING("No displays found.");

    currentDisplay = SDL_GetCurrentDisplayMode(displays[0]);

    // Creating window
    sdlWindow = SDL_CreateWindow(
      windowName,
      currentDisplay->w, currentDisplay->h,
      SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN
    );

    if (!sdlWindow) {
      LOG_CORE_ERROR("Failed to create window: {}", SDL_GetError());
      return;
    }

    SDL_SetWindowRelativeMouseMode(sdlWindow, mouseLocked);
  }

  Window::~Window() {
    SDL_DestroyWindow(sdlWindow);
    SDL_free(displays);
  }

  int Window::getWidth() const {
    return currentDisplay->w;
  }

  int Window::getHeight() const {
    return currentDisplay->h;
  }

  void Window::updateBuffer() const {
    SDL_GL_SwapWindow(sdlWindow);
  }

  void Window::unlockMouse() const {
    SDL_SetWindowRelativeMouseMode(sdlWindow, false);
  }

  void Window::lockMouse() const {
    SDL_SetWindowRelativeMouseMode(sdlWindow, true);
  }
}

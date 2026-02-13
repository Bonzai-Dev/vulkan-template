#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <core/logger.hpp>
#include "window.hpp"

namespace Core {
  Window::Window(const char *windowName, bool mouseLocked) {
    static bool sdlInitialized = false;
    if (!sdlInitialized) {
      if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_CORE_CRITICAL("Failed to initialize SDL Video: {}", SDL_GetError());
        return;
      }
      sdlInitialized = true;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
      LOG_CORE_CRITICAL("Failed to load Vulkan: {}", SDL_GetError());
      return;
    }

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

    // Creating OpenGL context
    // glContext = SDL_GL_CreateContext(sdlWindow);
    // const int version = gladLoadGL(SDL_GL_GetProcAddress);
    // if (!version) {
    //   logError(Context::Renderer, "Failed to initialize OpenGL context.");
    //   return;
    // }

    // logInfo(
    //   Context::Renderer,
    //   "Program has successfully initialized OpenGL %d.%d core profile.",
    //   GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)
    // );
  }

  Window::~Window() {
    destroy();
  }

  void Window::destroy() const {
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

#pragma once
#include <SDL3/SDL.h>

namespace Core {
  class Window {
    public:
      Window(const char *windowName, bool mouseLocked = true);

      ~Window();

      void destroy() const;

      void unlockMouse() const;

      void lockMouse() const;

      void updateBuffer() const;

      int getWidth() const;

      int getHeight() const;

    private:
      SDL_Window *sdlWindow;
      const SDL_DisplayMode *currentDisplay;
      SDL_DisplayID *displays;
  };
}

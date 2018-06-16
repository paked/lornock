#include <e/rect.hpp>

SDL_Rect Rect::toSDL(Rect r) {
  return SDL_Rect {
    (int) r.x,
    (int) r.y,
    (int) r.w,
    (int) r.h
  };
}


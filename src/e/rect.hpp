#pragma once

#include <SDL.h>

struct Rect {
  float x;
  float y;
  float w;
  float h;

  static SDL_Rect toSDL(Rect r);
};

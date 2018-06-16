#pragma once

#include <SDL.h>

struct Point {
  Point(float x = 0, float y = 0);

  float x = 0;
  float y = 0;

  static SDL_Point toSDL(Point p);
};

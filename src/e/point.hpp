#pragma once

#include <SDL.h>

struct Point {
  Point(float x = 0, float y = 0);

  float x = 0;
  float y = 0;

  float length();
  Point unit();

  static SDL_Point toSDL(Point p);
};

#include <e/point.hpp>

#include <math.h>

Point::Point(float x, float y) : x(x), y(y) {}

float Point::length() {
  return sqrt(x*x + y*y);
}

Point Point::unit() {
  Point p;

  float len = length();
  
  if (len == 0) {
    return p;
  }

  p.x = x/len;
  p.y = y/len;

  return p;
}

SDL_Point Point::toSDL(Point p) {
  return {
    (int) p.x,
    (int) p.y
  };
}

#include <e/point.hpp>

Point::Point(float x, float y) : x(x), y(y) {}

SDL_Point Point::toSDL(Point p) {
  return {
    (int) p.x,
    (int) p.y
  };
}

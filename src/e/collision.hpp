#pragma once

#include <e/tilemap.hpp>
#include <e/rect.hpp>
#include <e/point.hpp>

struct Collision {
  static void collide(Sprite* first, Sprite* second);
  static void collide(Sprite* first, Rect second);
  static void collide(Sprite* sprite, Tilemap* tilemap);

  static bool isOverlapping(Sprite* first, Rect second);
  static bool isOverlapping(Rect first, Rect second);
  static bool isOverlapping(Sprite* sprite, Tilemap* tilemap, int layer);

  static bool isOverlapping(Point p, Rect r);
  static bool intersection(Rect first, Rect second, Rect* result);
};

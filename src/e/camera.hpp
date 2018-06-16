#pragma once

#include <SDL.h>

#include <config.hpp>
#include <e/rect.hpp>
#include <e/point.hpp>

struct Sprite;

struct Camera {
  void tick(float dt);

  void shake(int duration, float trauma = 0.5);

  SDL_Rect toView(Rect rect, bool global = false, bool scale = true);
  Rect viewport(float buffer = 0);
  bool withinViewport(Rect rect);
  Point point();
  float getWidth();
  float getHeight();

  // Where the camera is heading
  Point getTarget();

  // Sprite to follow
  Sprite* follow = nullptr;
  // Point to follow if there is no dedicated sprite
  Point target = Point(0, 0);

  float zoom = 4;

  // Co-ordinate values which take into account shaking
  float x = 0;
  float y = 0;

  // Non-shook co-ordinates
  float realX = 0;
  float realY = 0;

  int logicalWidth = SCREEN_WIDTH;
  int logicalHeight = SCREEN_HEIGHT;

  float shakeTrauma = 0;
  float shakeMax = 25;
  int shakeStopTime = 0;
};

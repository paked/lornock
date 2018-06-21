#pragma once

#include <string>
#include <map>
#include <vector>

#include <SDL.h>

#include <e/point.hpp>
#include <e/rect.hpp>
#include <e/component.hpp>

#include <e/scene.hpp>

struct Sprite : Component {
  Sprite(std::string texName, float x=0, float y=0);

  void tick(float dt);
  void postTick();
  void render();

  void job(Scene* scene, float depth=0);

  Rect rect();

  Point getCenter();
  void center();

  virtual SDL_Rect getSRC();

  float x;
  float y;
  float width;
  float height;

  // should the image be flipped horizontally?
  bool flip = false;
  // render in camera space or on screen space?
  int angle = 0;
  unsigned char alpha = 255;

  bool hud = false;
  bool solid = true;
  bool visible = true;
  float localDepth = 0;

  Point velocity;
  Point acceleration;
  Point drag = Point { 0.99f, 0.99f };
  Point maxVelocity = Point { 1000.0f, 1000.0f };
  Point nextPositionDelta;

  int textureWidth;
  int textureHeight;
  SDL_Texture *texture;
};
